//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "moviedecoder.h"

#include "videoframe.h"

#include <cassert>
#include <array>
#include <sstream>
#include <memory>
#include <regex>

extern "C" {
#include <libavutil/display.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}


using namespace std;

namespace ffmpegthumbnailer
{

MovieDecoder::MovieDecoder(AVFormatContext* pavContext)
: m_VideoStream(-1)
, m_pFormatContext(pavContext)
, m_pVideoCodecContext(nullptr)
, m_pVideoCodec(nullptr)
, m_pFilterGraph(nullptr)
, m_pFilterSource(nullptr)
, m_pFilterSink(nullptr)
, m_pVideoStream(nullptr)
, m_pFrame(nullptr)
, m_pPacket(nullptr)
, m_FormatContextWasGiven(pavContext != nullptr)
, m_AllowSeek(true)
, m_UseEmbeddedData(false)
{
}

MovieDecoder::~MovieDecoder()
{
    destroy();
}

void MovieDecoder::initialize(const string& filename, bool preferEmbeddedMetadata)
{
    avformat_network_init();

    string inputFile = filename == "-" ? "pipe:" : filename;
    m_AllowSeek = (filename != "-") && (filename.find("rtsp://") != 0) && (filename.find("udp://") != 0);

    if ((!m_FormatContextWasGiven) && avformat_open_input(&m_pFormatContext, inputFile.c_str(), nullptr, nullptr) != 0)
    {
        destroy();
        throw logic_error(string("Could not open input file: ") + filename);
    }

    if (avformat_find_stream_info(m_pFormatContext, nullptr) < 0)
    {
        destroy();
        throw logic_error("Could not find stream information");
    }

    initializeVideo(preferEmbeddedMetadata);
    m_pFrame = av_frame_alloc();
}

void MovieDecoder::destroy()
{
    if (m_pVideoCodecContext)
    {
        avcodec_free_context(&m_pVideoCodecContext);
    }

    if ((!m_FormatContextWasGiven) && m_pFormatContext)
    {
        avformat_close_input(&m_pFormatContext);
    }

    if (m_pPacket)
    {
        av_packet_unref(m_pPacket);
        delete m_pPacket;
        m_pPacket = nullptr;
    }

    if (m_pFrame)
    {
        av_frame_free(&m_pFrame);
    }

    m_VideoStream = -1;

    avformat_network_deinit();
}

bool MovieDecoder::embeddedMetaDataIsAvailable()
{
    return m_UseEmbeddedData;
}

string MovieDecoder::getCodec()
{
    if (m_pVideoCodec)
    {
        return m_pVideoCodec->name;
    }

    return "";
}

static bool isStillImageCodec(AVCodecID codecId)
{
    return     codecId == AV_CODEC_ID_MJPEG
            || codecId == AV_CODEC_ID_PNG;
}

int32_t MovieDecoder::findPreferedVideoStream(bool preferEmbeddedMetadata)
{
    std::vector<int32_t> videoStreams;
    std::vector<int32_t> embeddedDataStream;

    for (unsigned int i = 0; i < m_pFormatContext->nb_streams; ++i)
    {
        AVStream *stream = m_pFormatContext->streams[i];
        auto par = m_pFormatContext->streams[i]->codecpar;
        if (par->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (!preferEmbeddedMetadata || !isStillImageCodec(par->codec_id))
            {
                videoStreams.push_back(i);
                continue;
            }

            if (stream->metadata)
            {
                AVDictionaryEntry* tag = nullptr;
                while ((tag = av_dict_get(stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
                {
                    if (strcmp(tag->key, "filename") == 0 && strncmp(tag->value, "cover.", 6) == 0)
                    {
                        embeddedDataStream.insert(embeddedDataStream.begin(), i);
                        continue;
                    }
                }
            }

            embeddedDataStream.push_back(i);
        }
    }

    m_UseEmbeddedData = false;
    if (preferEmbeddedMetadata && !embeddedDataStream.empty())
    {
        m_UseEmbeddedData = true;
        return embeddedDataStream.front();
    }
    else if (!videoStreams.empty())
    {
        return videoStreams.front();
    }

    return -1;
}

void MovieDecoder::initializeVideo(bool preferEmbeddedMetadata)
{
    m_VideoStream = findPreferedVideoStream(preferEmbeddedMetadata);

    if (m_VideoStream < 0)
    {
        destroy();
        throw logic_error("Could not find video stream");
    }

    m_pVideoStream = m_pFormatContext->streams[m_VideoStream];
    m_pVideoCodec = avcodec_find_decoder(m_pVideoStream->codecpar->codec_id);

    if (m_pVideoCodec == nullptr)
    {
        // set to nullptr, otherwise avcodec_close(m_pVideoCodecContext) crashes
        m_pVideoCodecContext = nullptr;
        destroy();
        throw logic_error("Video Codec not found");
    }

    m_pVideoCodecContext = avcodec_alloc_context3(m_pVideoCodec);

    if (m_pVideoCodecContext == nullptr)
    {
        destroy();
        throw logic_error("Could not allocate video codec context");
    }

    if (avcodec_parameters_to_context(m_pVideoCodecContext, m_pVideoStream->codecpar) < 0)
    {
        destroy();
        throw logic_error("Could not configure video codec context");
    }

    m_pVideoCodecContext->workaround_bugs = 1;

    if (avcodec_open2(m_pVideoCodecContext, m_pVideoCodec, nullptr) < 0)
    {
        destroy();
        throw logic_error("Could not open video codec");
    }
}

std::string MovieDecoder::createScaleString(const std::string& sizeString, bool maintainAspectRatio)
{
    int width = -1;
    int height = -1;
    bool pureNumericSize = true;

    if (sizeString.empty())
    {
        return "w=0:h=0";
    }

    try
    {
        std::regex sizeRegex(R"r(([w|h])=(-?\d+)(?::([w|h])=(-?\d+))?)r");
        std::smatch baseMatch;
        if (std::regex_match(sizeString, baseMatch, sizeRegex))
        {
            if (baseMatch.size() != 3 && baseMatch.size() != 5)
            {
                throw std::runtime_error("Failed to parse size string");
            }

            auto parseMatch = [&width, &height] (std::smatch& match, size_t index) {
                auto specifier = match[index].str();
                if (specifier == "w")
                {
                    width = std::stoi(match[index+1].str());
                    if (width <= 0)
                    {
                        width = -1;
                    }
                }
                else if (specifier == "h")
                {
                    height = std::stoi(match[index+1].str());
                    if (height <= 0)
                    {
                        height = -1;
                    }
                }
            };

            pureNumericSize = false;
            if (baseMatch.size() >= 3)
            {
                parseMatch(baseMatch, 1);
            }

            if (baseMatch.size() == 5)
            {
                parseMatch(baseMatch, 3);
            }
        }
        else
        {
            width = std::stoi(sizeString);
        }
    }
    catch (const std::regex_error&)
    {
        throw std::runtime_error("Failed to parse size string");
    }

    std::stringstream scale;

    if (width != -1 && height != -1)
    {
        scale << "w=" << width << ":h=" << height;
        if (maintainAspectRatio)
        {
            scale << ":force_original_aspect_ratio=decrease";
        }
    }
    else if (!maintainAspectRatio)
    {
        if (width == -1)
        {
            scale << "w=" << height << ":h=" << height;
        }
        else
        {
            scale << "w=" << width << ":h=" << width;
        }
    }
    else
    {
        auto size = (height == -1) ? width : height;

        width      = m_pVideoCodecContext->width;
        height     = m_pVideoCodecContext->height;

        AVRational par = av_guess_sample_aspect_ratio(m_pFormatContext, m_pVideoStream, m_pFrame);
        // if the pixel aspect ratio is defined and is not 1, we have an anamorphic stream
        bool anamorphic = par.num != 0 && par.num != par.den;

        if (anamorphic)
        {
            width = width * par.num / par.den;

            if (size)
            {
                if (pureNumericSize)
                {
                    if (height > width)
                    {
                        width = width * size / height;
                        height = size;
                    }
                    else
                    {
                        height = height * size / width;
                        width = size;
                    }
                }
                else
                {
                    if (sizeString[0] == 'h')
                    {
                        width = width * size / height;
                        height = size;
                    }
                    else
                    {
                        height = height * size / width;
                        width = size;
                    }
                }
            }

            scale << "w=" << width << ":h=" << height;
        }
        else
        {
            if (pureNumericSize)
            {
                if (height > width)
                {
                    scale << "w=-1:h=" << (size == 0 ? height : size);
                }
                else
                {
                    scale << "h=-1:w=" << (size == 0 ? width : size);
                }
            }
            else
            {
                if (sizeString[0] == 'w')
                {
                    scale << "h=-1:w=" << (size == 0 ? width : size);
                }
                else
                {
                    scale << "w=-1:h=" << (size == 0 ? height : size);
                }
            }
        }
    }

    return scale.str();
}

void MovieDecoder::initializeFilterGraph(const AVRational& timeBase, const std::string& size, bool maintainAspectRatio)
{
    m_pFilterGraph = avfilter_graph_alloc();
    assert(m_pFilterGraph);

    std::stringstream ss;
    ss << "video_size=" << m_pVideoCodecContext->width << "x" << m_pVideoCodecContext->height
       << ":pix_fmt=" << m_pVideoCodecContext->pix_fmt
       << ":time_base=" << timeBase.num << "/" << timeBase.den
       << ":pixel_aspect=" << m_pVideoCodecContext->sample_aspect_ratio.num << "/" << FFMAX(m_pVideoCodecContext->sample_aspect_ratio.den, 1);

    checkRc(avfilter_graph_create_filter(&m_pFilterSource, avfilter_get_by_name("buffer"), "thumb_buffer", ss.str().c_str(), nullptr, m_pFilterGraph),
            "Failed to create filter source");
    checkRc(avfilter_graph_create_filter(&m_pFilterSink, avfilter_get_by_name("buffersink"), "thumb_buffersink", nullptr, nullptr, m_pFilterGraph),
            "Failed to create filter sink");

    AVFilterContext* yadifFilter = nullptr;
    if (m_pFrame->interlaced_frame != 0)
    {
        checkRc(avfilter_graph_create_filter(&yadifFilter, avfilter_get_by_name("yadif"), "thumb_deint", "deint=1", nullptr, m_pFilterGraph),
                "Failed to create deinterlace filter");
    }

    AVFilterContext* scaleFilter = nullptr;
    checkRc(avfilter_graph_create_filter(&scaleFilter, avfilter_get_by_name("scale"), "thumb_scale", createScaleString(size, maintainAspectRatio).c_str(), nullptr, m_pFilterGraph),
            "Failed to create scale filter");

    AVFilterContext* formatFilter = nullptr;
    checkRc(avfilter_graph_create_filter(&formatFilter, avfilter_get_by_name("format"), "thumb_format", "pix_fmts=rgb24", nullptr, m_pFilterGraph),
            "Failed to create format filter");

    AVFilterContext* rotateFilter = nullptr;
    auto rotation = getStreamRotation();
    if (rotation == 3)
    {
        checkRc(avfilter_graph_create_filter(&rotateFilter, avfilter_get_by_name("rotate"), "thumb_rotate", "PI", nullptr, m_pFilterGraph),
            "Failed to create rotate filter");
    }
    else if (rotation != -1)
    {
        checkRc(avfilter_graph_create_filter(&rotateFilter, avfilter_get_by_name("transpose"), "thumb_transpose", std::to_string(rotation).c_str(), nullptr, m_pFilterGraph),
            "Failed to create rotate filter");
    }

    checkRc(avfilter_link(rotateFilter ? rotateFilter : formatFilter, 0, m_pFilterSink, 0), "Failed to link final filter");

    if (rotateFilter)
    {
        checkRc(avfilter_link(formatFilter, 0, rotateFilter, 0), "Failed to link format filter");
    }

    checkRc(avfilter_link(scaleFilter, 0, formatFilter, 0), "Failed to link scale filter");

    if (yadifFilter)
    {
        checkRc(avfilter_link(yadifFilter, 0, scaleFilter, 0), "Failed to link yadif filter");
    }

    checkRc(avfilter_link(m_pFilterSource, 0, yadifFilter ? yadifFilter : scaleFilter, 0), "Failed to link source filter");
    checkRc(avfilter_graph_config(m_pFilterGraph, nullptr), "Failed to configure filter graph");
}

int MovieDecoder::getWidth()
{
    if (m_pVideoCodecContext)
    {
        return m_pVideoCodecContext->width;
    }

    return -1;
}

int MovieDecoder::getHeight()
{
    if (m_pVideoCodecContext)
    {
        return m_pVideoCodecContext->height;
    }

    return -1;
}

int MovieDecoder::getDuration()
{
    if (m_pFormatContext)
    {
        return static_cast<int>(m_pFormatContext->duration / AV_TIME_BASE);
    }

    return 0;
}

void MovieDecoder::seek(int timeInSeconds)
{
    if (!m_AllowSeek)
    {
        return;
    }

    int64_t timestamp = AV_TIME_BASE * static_cast<int64_t>(timeInSeconds);

    if (timestamp < 0)
    {
        timestamp = 0;
    }

    checkRc(av_seek_frame(m_pFormatContext, -1, timestamp, 0), "Seeking in video failed");
    avcodec_flush_buffers(m_pVideoCodecContext);

    int keyFrameAttempts = 0;
    bool gotFrame;

    do
    {
        int count = 0;
        gotFrame = false;

        while (!gotFrame && count < 20)
        {
            getVideoPacket();
            try
            {
                gotFrame = decodeVideoPacket();
            }
            catch(logic_error&) {}
            ++count;
        }

        ++keyFrameAttempts;
    } while ((!gotFrame || !m_pFrame->key_frame) && keyFrameAttempts < 200);

    if (gotFrame == 0)
    {
        throw logic_error("Seeking in video failed");
    }
}

void MovieDecoder::decodeVideoFrame()
{
    bool frameFinished = false;

    while (!frameFinished && getVideoPacket())
    {
        frameFinished = decodeVideoPacket();
    }

    if (!frameFinished)
    {
        throw logic_error("decodeVideoFrame() failed: frame not finished");
    }
}

bool MovieDecoder::decodeVideoPacket()
{
    if (m_pPacket->stream_index != m_VideoStream)
    {
        return false;
    }

    int rc = avcodec_send_packet(m_pVideoCodecContext, m_pPacket);
    if(rc == AVERROR(EAGAIN))
    {
        rc = 0;
    }

    if(rc == AVERROR_EOF)
    {
        return false;
    }
    else if(rc < 0)
    {
        throw logic_error("Failed to decode video frame: avcodec_send_packet() < 0");
    }

    rc = avcodec_receive_frame(m_pVideoCodecContext, m_pFrame);
    switch(rc)
    {
        case 0:
            return true;

        case AVERROR(EAGAIN):
            return false;

        default:
            throw logic_error("Failed to decode video frame: avcodec_receive_frame() < 0");
    }
}

bool MovieDecoder::getVideoPacket()
{
    bool framesAvailable = true;
    bool frameDecoded = false;

    if (m_pPacket)
    {
        av_packet_unref(m_pPacket);
        delete m_pPacket;
    }

    m_pPacket = new AVPacket();


    while (framesAvailable && !frameDecoded)
    {
        framesAvailable = av_read_frame(m_pFormatContext, m_pPacket) >= 0;
        if (framesAvailable)
        {
            frameDecoded = m_pPacket->stream_index == m_VideoStream;
            if (!frameDecoded)
            {
                av_packet_unref(m_pPacket);
            }
        }
    }

    return frameDecoded;
}

void MovieDecoder::getScaledVideoFrame(const std::string& scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame)
{
    initializeFilterGraph(m_pFormatContext->streams[m_VideoStream]->time_base, scaledSize, maintainAspectRatio);

    auto del = [] (AVFrame* f) { av_frame_free(&f); };
    std::unique_ptr<AVFrame, decltype(del)> res(av_frame_alloc(), del);

    checkRc(av_buffersrc_write_frame(m_pFilterSource, m_pFrame), "Failed to write frame to filter graph");

    int attempts = 0;
    int rc = av_buffersink_get_frame(m_pFilterSink, res.get());
    while (rc == AVERROR(EAGAIN) && attempts++ < 10)
    {
        decodeVideoFrame();
        checkRc(av_buffersrc_write_frame(m_pFilterSource, m_pFrame), "Failed to write frame to filter graph");
        rc = av_buffersink_get_frame(m_pFilterSink, res.get());
    }

    checkRc(rc, "Failed to get buffer from filter");

    videoFrame.width = res->width;
    videoFrame.height = res->height;
    videoFrame.lineSize = res->linesize[0];
    videoFrame.imageSource = m_UseEmbeddedData ? ThumbnailerImageSourceMetadata : ThumbnailerImageSourceVideoStream;

    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((videoFrame.frameData.data()), res->data[0], videoFrame.frameData.size());

    if (m_pFilterGraph)
    {
        avfilter_graph_free(&m_pFilterGraph);
    }
}

void MovieDecoder::checkRc(int ret, const std::string& message)
{
    if (ret < 0)
    {
        char buf[256];
        buf[0] = ' ';
        av_strerror(ret, &buf[1], sizeof(buf) - 1);
        throw std::logic_error(message + buf);
    }
}


int32_t MovieDecoder::getStreamRotation()
{
    auto matrix = reinterpret_cast<int32_t*>(av_stream_get_side_data(m_pVideoStream, AV_PKT_DATA_DISPLAYMATRIX, nullptr));
    if (matrix)
    {
        auto angle = lround(av_display_rotation_get(matrix));
        if (angle < -135)
        {
            return 3;
        }
        else if (angle > 45 && angle < 135)
        {
            return 2;
        }
        else if (angle < -45 && angle > -135)
        {
            return 1;
        }
    }

    return -1;
}

}

