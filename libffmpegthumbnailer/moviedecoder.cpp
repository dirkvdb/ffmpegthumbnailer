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

#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <array>
#include <sstream>
#include <memory>

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

using namespace std;

namespace ffmpegthumbnailer
{

struct SilenceLogLevel
{
    SilenceLogLevel() { av_log_set_level(AV_LOG_QUIET); }
};

MovieDecoder::MovieDecoder(const string& filename, AVFormatContext* pavContext)
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
{
    initialize(filename);
}

MovieDecoder::~MovieDecoder()
{
    destroy();
}

void MovieDecoder::initialize(const string& filename)
{
    av_register_all();
    avcodec_register_all();
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

    initializeVideo();
    m_pFrame = av_frame_alloc();
}

void MovieDecoder::destroy()
{
    if (m_pVideoCodecContext)
    {
        avcodec_close(m_pVideoCodecContext);
        m_pVideoCodecContext = nullptr;
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

string MovieDecoder::getCodec()
{
    if (m_pVideoCodec)
    {
        return m_pVideoCodec->name;
    }

    return "";
}

void MovieDecoder::initializeVideo()
{
    for (unsigned int i = 0; i < m_pFormatContext->nb_streams; ++i)
    {
        if (m_pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_pVideoStream = m_pFormatContext->streams[i];
            m_VideoStream = i;
            break;
        }
    }

    if (m_VideoStream == -1)
    {
        destroy();
        throw logic_error("Could not find video stream");
    }

    m_pVideoCodecContext = m_pFormatContext->streams[m_VideoStream]->codec;
    m_pVideoCodec = avcodec_find_decoder(m_pVideoCodecContext->codec_id);

    if (m_pVideoCodec == nullptr)
    {
        // set to nullptr, otherwise avcodec_close(m_pVideoCodecContext) crashes
        m_pVideoCodecContext = nullptr;
        destroy();
        throw logic_error("Video Codec not found");
    }

    m_pVideoCodecContext->workaround_bugs = 1;

    if (avcodec_open2(m_pVideoCodecContext, m_pVideoCodec, nullptr) < 0)
    {
        destroy();
        throw logic_error("Could not open video codec");
    }
}

void MovieDecoder::initializeFilterGraph(AVRational timeBase, int width, int height)
{
    static const AVPixelFormat pixelFormats[] = { AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE };

    auto del = [] (AVBufferSinkParams* p) { av_freep(p); };
    std::unique_ptr<AVBufferSinkParams, decltype(del)> buffersinkParams(av_buffersink_params_alloc(), del);

    avfilter_register_all();

    m_pFilterGraph = avfilter_graph_alloc();
    assert(m_pFilterGraph);

    std::stringstream ss;
    ss << "video_size=" << m_pVideoCodecContext->width << "x" << m_pVideoCodecContext->height
       << ":pix_fmt=" << m_pVideoCodecContext->pix_fmt
       << ":time_base=" << timeBase.num << "/" << timeBase.den
       << ":pixel_aspect=" << m_pVideoCodecContext->sample_aspect_ratio.num << "/" << FFMAX(m_pVideoCodecContext->sample_aspect_ratio.den, 1);

    checkRc(avfilter_graph_create_filter(&m_pFilterSource, avfilter_get_by_name("buffer"), "thumb_buffer", ss.str().c_str(), nullptr, m_pFilterGraph),
            "Failed to create filter source");
    buffersinkParams->pixel_fmts = pixelFormats;
    checkRc(avfilter_graph_create_filter(&m_pFilterSink, avfilter_get_by_name("buffersink"), "thumb_buffersink", nullptr, buffersinkParams.get(), m_pFilterGraph),
            "Failed to create filter sink");
    buffersinkParams.release();

    AVFilterContext* yadifFilter = nullptr;
    checkRc(avfilter_graph_create_filter(&yadifFilter, avfilter_get_by_name("yadif"), "thumb_deint", "deint=1", nullptr, m_pFilterGraph),
            "Failed to create deinterlace filter");

    std::stringstream scale;
    scale << "w=" << width
          << ":h=" << height
          << ":sws_flags=bicubic";

    AVFilterContext* scaleFilter = nullptr;
    checkRc(avfilter_graph_create_filter(&scaleFilter, avfilter_get_by_name("scale"), "thumb_scale", scale.str().c_str(), nullptr, m_pFilterGraph),
            "Failed to create scale filter");

    AVFilterContext* formatFilter = nullptr;
    checkRc(avfilter_graph_create_filter(&formatFilter, avfilter_get_by_name("format"), "thumb_format", "pix_fmts=rgb24", nullptr, m_pFilterGraph),
            "Failed to create scale filter");

    checkRc(avfilter_link(formatFilter, 0, m_pFilterSink, 0), "Failed to link filters");
    checkRc(avfilter_link(scaleFilter, 0, formatFilter, 0), "Failed to link filters");
    checkRc(avfilter_link(yadifFilter, 0, scaleFilter, 0), "Failed to link filters");
    checkRc(avfilter_link(m_pFilterSource, 0, yadifFilter, 0), "Failed to link filters");

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
    avcodec_flush_buffers(m_pFormatContext->streams[m_VideoStream]->codec);

    int keyFrameAttempts = 0;
    bool gotFrame = 0;

    do
    {
        int count = 0;
        gotFrame = 0;

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

    av_frame_unref(m_pFrame);

    int frameFinished;

    int bytesDecoded = avcodec_decode_video2(m_pVideoCodecContext, m_pFrame, &frameFinished, m_pPacket);
    if (bytesDecoded < 0)
    {
        throw logic_error("Failed to decode video frame: bytesDecoded < 0");
    }

    return frameFinished > 0;
}

bool MovieDecoder::getVideoPacket()
{
    bool framesAvailable = true;
    bool frameDecoded = false;

    int attempts = 0;

    if (m_pPacket)
    {
        av_packet_unref(m_pPacket);
        delete m_pPacket;
    }

    m_pPacket = new AVPacket();

    while (framesAvailable && !frameDecoded && (attempts++ < 1000))
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

void MovieDecoder::getScaledVideoFrame(int scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame)
{
    int scaledWidth = 0;
    int scaledHeight = 0;
    calculateDimensions(scaledSize, maintainAspectRatio, scaledWidth, scaledHeight);

    initializeFilterGraph(m_pFormatContext->streams[m_VideoStream]->time_base, scaledWidth, scaledHeight);

    checkRc(av_buffersrc_write_frame(m_pFilterSource, m_pFrame), "Failed to write frame tp filter graph");
    decodeVideoFrame();
    checkRc(av_buffersrc_write_frame(m_pFilterSource, m_pFrame), "Failed to write frame tp filter graph");

    auto del = [] (AVFrame* f) { av_frame_free(&f); };
    std::unique_ptr<AVFrame, decltype(del)> res(av_frame_alloc(), del);
    checkRc(av_buffersink_get_frame(m_pFilterSink, res.get()), "Failed to get buffer from filter");

    videoFrame.width = res->width;
    videoFrame.height = res->height;
    videoFrame.lineSize = res->linesize[0];

    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((videoFrame.frameData.data()), res->data[0], videoFrame.frameData.size());


    if (m_pFilterGraph)
    {
        avfilter_graph_free(&m_pFilterGraph);
    }
}

void MovieDecoder::calculateDimensions(int squareSize, bool maintainAspectRatio, int& destWidth, int& destHeight)
{
    AVRational par = av_guess_sample_aspect_ratio(m_pFormatContext, m_pVideoStream, m_pFrame);

    // if the pixel aspect ratio is defined and is not 1, we have an anamorphic stream
    bool anamorphic = par.num != 0 && par.num != par.den;

    if (squareSize == 0)
    {
        // use original video size
        squareSize = max(m_pVideoCodecContext->width, m_pVideoCodecContext->height);
    }

    if (!maintainAspectRatio)
    {
        destWidth = squareSize;
        destHeight = squareSize;
    }
    else
    {
        int srcWidth            = m_pVideoCodecContext->width;
        int srcHeight           = m_pVideoCodecContext->height;

        if (anamorphic)
        {
            srcWidth = srcWidth * par.num / par.den;
        }

        if (srcWidth > srcHeight)
        {
            destWidth  = squareSize;
            destHeight = static_cast<int>(static_cast<float>(squareSize) / srcWidth * srcHeight);
        }
        else
        {
            destWidth  = static_cast<int>(static_cast<float>(squareSize) / srcHeight * srcWidth);
            destHeight = squareSize;
        }
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

}

