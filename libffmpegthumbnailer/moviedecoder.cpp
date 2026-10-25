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
#include <memory>
#include <regex>
#include <sstream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/display.h>
#include <libavutil/dovi_meta.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

using namespace std;

namespace ffmpegthumbnailer {

namespace {

class AvDictionaryWrapper
{
public:
    AvDictionaryWrapper() = default;
    ~AvDictionaryWrapper() { av_dict_free(&m_Dictionary); }

    AvDictionaryWrapper(const AvDictionaryWrapper&) = delete;
    AvDictionaryWrapper& operator=(const AvDictionaryWrapper&) = delete;

    int set(const char* key, const char* value)
    {
        return av_dict_set(&m_Dictionary, key, value, 0);
    }

    AVDictionary** address() { return &m_Dictionary; }

private:
    AVDictionary* m_Dictionary = nullptr;
};

}

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

void MovieDecoder::setAllowedProtocols(std::string_view protocols)
{
    m_AllowedProtocols = protocols;
}

void MovieDecoder::initialize(const string& filename, bool preferEmbeddedMetadata)
{
    avformat_network_init();

    string inputFile = filename == "-" ? "pipe:" : filename;
    m_AllowSeek      = (filename != "-") && (filename.find("rtsp://") != 0) && (filename.find("udp://") != 0);

    if (!m_FormatContextWasGiven) {
        AvDictionaryWrapper options;
        int ret = 0;
        if (m_AllowedProtocols != "ALL") {
            ret = options.set("protocol_whitelist", m_AllowedProtocols.c_str());
        }

        if (ret >= 0) {
            ret = avformat_open_input(&m_pFormatContext, inputFile.c_str(), nullptr, options.address());
        }

        if (ret < 0) {
            destroy();
            throw logic_error(string("Could not open input file: ") + filename);
        }
    }

    if (avformat_find_stream_info(m_pFormatContext, nullptr) < 0) {
        destroy();
        throw logic_error("Could not find stream information");
    }

    initializeVideo(preferEmbeddedMetadata);
    m_pFrame = av_frame_alloc();
}

void MovieDecoder::destroy()
{
    avfilter_graph_free(&m_pFilterGraph);
    m_pFilterSource = nullptr;
    m_pFilterSink = nullptr;

    if (m_pVideoCodecContext) {
        avcodec_free_context(&m_pVideoCodecContext);
    }

    if ((!m_FormatContextWasGiven) && m_pFormatContext) {
        avformat_close_input(&m_pFormatContext);
    }

    if (m_pPacket) {
        av_packet_unref(m_pPacket);
        delete m_pPacket;
        m_pPacket = nullptr;
    }

    if (m_pFrame) {
        av_frame_free(&m_pFrame);
    }

    m_VideoStream = -1;

    avformat_network_deinit();
}

bool MovieDecoder::embeddedMetaDataIsAvailable() const
{
    return m_UseEmbeddedData;
}

string MovieDecoder::getCodec() const
{
    if (m_pVideoCodec) {
        return m_pVideoCodec->name;
    }

    return "";
}

static bool isStillImageCodec(AVCodecID codecId)
{
    return codecId == AV_CODEC_ID_MJPEG || codecId == AV_CODEC_ID_PNG;
}

int32_t MovieDecoder::findPreferredVideoStream(bool preferEmbeddedMetadata)
{
    std::vector<int32_t> videoStreams;
    std::vector<int32_t> embeddedDataStream;

    for (unsigned int i = 0; i < m_pFormatContext->nb_streams; ++i) {
        AVStream* stream = m_pFormatContext->streams[i];
        auto par         = m_pFormatContext->streams[i]->codecpar;
        if (par->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (!preferEmbeddedMetadata || !isStillImageCodec(par->codec_id)) {
                videoStreams.push_back(i);
                continue;
            }

            if (stream->metadata) {
                AVDictionaryEntry* tag = nullptr;
                while ((tag = av_dict_get(stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
                    if (strcmp(tag->key, "filename") == 0 && strncmp(tag->value, "cover.", 6) == 0) {
                        embeddedDataStream.insert(embeddedDataStream.begin(), i);
                        continue;
                    }
                }
            }

            embeddedDataStream.push_back(i);
        }
    }

    m_UseEmbeddedData = false;
    if (preferEmbeddedMetadata && !embeddedDataStream.empty()) {
        m_UseEmbeddedData = true;
        return embeddedDataStream.front();
    } else if (!videoStreams.empty()) {
        return videoStreams.front();
    }

    return -1;
}

void MovieDecoder::initializeVideo(bool preferEmbeddedMetadata)
{
    m_VideoStream = findPreferredVideoStream(preferEmbeddedMetadata);

    if (m_VideoStream < 0) {
        destroy();
        throw logic_error("Could not find video stream");
    }

    m_pVideoStream = m_pFormatContext->streams[m_VideoStream];
    m_pVideoCodec  = avcodec_find_decoder(m_pVideoStream->codecpar->codec_id);

    if (m_pVideoCodec == nullptr) {
        // set to nullptr, otherwise avcodec_close(m_pVideoCodecContext) crashes
        m_pVideoCodecContext = nullptr;
        destroy();
        throw logic_error("Video Codec not found");
    }

    m_pVideoCodecContext = avcodec_alloc_context3(m_pVideoCodec);

    if (m_pVideoCodecContext == nullptr) {
        destroy();
        throw logic_error("Could not allocate video codec context");
    }

    if (avcodec_parameters_to_context(m_pVideoCodecContext, m_pVideoStream->codecpar) < 0) {
        destroy();
        throw logic_error("Could not configure video codec context");
    }

    m_pVideoCodecContext->workaround_bugs = 1;

    if (avcodec_open2(m_pVideoCodecContext, m_pVideoCodec, nullptr) < 0) {
        destroy();
        throw logic_error("Could not open video codec");
    }
}

std::string MovieDecoder::createScaleString(const std::string& sizeString, bool maintainAspectRatio)
{
    int width            = -1;
    int height           = -1;
    bool pureNumericSize = true;

    if (sizeString.empty()) {
        return "w=0:h=0";
    }

    try {
        std::regex sizeRegex(R"r(([w|h])=(-?\d+)(?::([w|h])=(-?\d+))?)r");
        std::smatch baseMatch;
        if (std::regex_match(sizeString, baseMatch, sizeRegex)) {
            if (baseMatch.size() != 3 && baseMatch.size() != 5) {
                throw std::runtime_error("Failed to parse size string");
            }

            auto parseMatch = [&width, &height](std::smatch& match, size_t index) {
                auto specifier = match[index].str();
                if (specifier == "w") {
                    width = std::stoi(match[index + 1].str());
                    if (width <= 0) {
                        width = -1;
                    }
                } else if (specifier == "h") {
                    height = std::stoi(match[index + 1].str());
                    if (height <= 0) {
                        height = -1;
                    }
                }
            };

            pureNumericSize = false;
            if (baseMatch.size() >= 3) {
                parseMatch(baseMatch, 1);
            }

            if (baseMatch.size() == 5) {
                parseMatch(baseMatch, 3);
            }
        } else {
            width = std::stoi(sizeString);
        }
    } catch (const std::regex_error&) {
        throw std::runtime_error("Failed to parse size string");
    }

    std::stringstream scale;

    if (width != -1 && height != -1) {
        scale << "w=" << width << ":h=" << height;
        if (maintainAspectRatio) {
            scale << ":force_original_aspect_ratio=decrease";
        }
    } else if (!maintainAspectRatio) {
        if (width == -1) {
            scale << "w=" << height << ":h=" << height;
        } else {
            scale << "w=" << width << ":h=" << width;
        }
    } else {
        auto size = (height == -1) ? width : height;

        width  = m_pVideoCodecContext->width;
        height = m_pVideoCodecContext->height;

        AVRational par = av_guess_sample_aspect_ratio(m_pFormatContext, m_pVideoStream, m_pFrame);
        // if the pixel aspect ratio is defined and is not 1, we have an anamorphic stream
        bool anamorphic = par.num != 0 && par.num != par.den;

        if (anamorphic) {
            width = width * par.num / par.den;

            if (size) {
                if (pureNumericSize) {
                    if (height > width) {
                        width  = width * size / height;
                        height = size;
                    } else {
                        height = height * size / width;
                        width  = size;
                    }
                } else {
                    if (sizeString[0] == 'h') {
                        width  = width * size / height;
                        height = size;
                    } else {
                        height = height * size / width;
                        width  = size;
                    }
                }
            }

            scale << "w=" << width << ":h=" << height;
        } else {
            if (pureNumericSize) {
                if (height > width) {
                    scale << "w=-1:h=" << (size == 0 ? height : size);
                } else {
                    scale << "h=-1:w=" << (size == 0 ? width : size);
                }
            } else {
                if (sizeString[0] == 'w') {
                    scale << "h=-1:w=" << (size == 0 ? width : size);
                } else {
                    scale << "w=-1:h=" << (size == 0 ? height : size);
                }
            }
        }
    }

    return scale.str();
}

bool MovieDecoder::needsDeinterlacing() const
{
    // PNG/APNG's Adam7 interlacing is spatial, not temporal; it must not go through yadif.
    return (m_pFrame->flags & AV_FRAME_FLAG_INTERLACED) &&
           m_pVideoCodecContext->codec_id != AV_CODEC_ID_PNG && m_pVideoCodecContext->codec_id != AV_CODEC_ID_APNG;
}

void MovieDecoder::initializeFilterGraph(const AVRational& timeBase, const std::string& size, bool maintainAspectRatio, ToneMapping toneMapping)
{
    avfilter_graph_free(&m_pFilterGraph);
    m_pFilterSource = nullptr;
    m_pFilterSink = nullptr;
    m_pFilterGraph = avfilter_graph_alloc();
    if (!m_pFilterGraph) {
        throw std::bad_alloc();
    }

    std::stringstream ss;
    const auto aspect = m_pFrame->sample_aspect_ratio;
    ss << "video_size=" << m_pFrame->width << "x" << m_pFrame->height
       << ":pix_fmt=" << m_pFrame->format
       << ":time_base=" << timeBase.num << "/" << timeBase.den
       << ":pixel_aspect=" << aspect.num << "/" << FFMAX(aspect.den, 1);

#if LIBAVFILTER_VERSION_INT >= AV_VERSION_INT(10, 1, 100)
    if (toneMapping != ToneMapping::None) {
        ss << ":colorspace=" << m_pFrame->colorspace << ":range=" << m_pFrame->color_range;
    }
#endif
    if (toneMapping == ToneMapping::DolbyVision) {
        ss << createDolbyVisionSourceArgs();
    }

    checkRc(avfilter_graph_create_filter(&m_pFilterSource, avfilter_get_by_name("buffer"), "thumb_buffer", ss.str().c_str(), nullptr, m_pFilterGraph),
            "Failed to create filter source");
    checkRc(avfilter_graph_create_filter(&m_pFilterSink, avfilter_get_by_name("buffersink"), "thumb_buffersink", nullptr, nullptr, m_pFilterGraph),
            "Failed to create filter sink");

    AVFilterContext* previous = m_pFilterSource;
    if (needsDeinterlacing()) {
        previous = appendFilter(previous, "yadif", "thumb_deint", "deint=1");
    }

    previous = appendToneMappingFilters(previous, toneMapping);
    previous = appendFilter(previous, "scale", "thumb_scale", createScaleString(size, maintainAspectRatio));
    previous = appendFilter(previous, "format", "thumb_format", "pix_fmts=rgb24");

    auto rotation = getStreamRotation();
    if (rotation == 3) {
        previous = appendFilter(previous, "rotate", "thumb_rotate", "PI");
    } else if (rotation != -1) {
        previous = appendFilter(previous, "transpose", "thumb_transpose", std::to_string(rotation));
    }

    checkRc(avfilter_link(previous, 0, m_pFilterSink, 0), "Failed to link final filter");
    checkRc(avfilter_graph_config(m_pFilterGraph, nullptr), "Failed to configure filter graph");
}

AVFilterContext* MovieDecoder::appendFilter(AVFilterContext* previous, const char* filterName, const char* name, const std::string& args)
{
    const AVFilter* filter = avfilter_get_by_name(filterName);
    if (!filter) {
        throw logic_error(std::string("Required FFmpeg filter is unavailable: ") + filterName);
    }
    AVFilterContext* context = nullptr;
    checkRc(avfilter_graph_create_filter(&context, filter, name, args.c_str(), nullptr, m_pFilterGraph),
            std::string("Failed to create filter: ") + filterName);
    checkRc(avfilter_link(previous, 0, context, 0), std::string("Failed to link filter: ") + filterName);
    return context;
}

AVFilterContext* MovieDecoder::appendToneMappingFilters(AVFilterContext* previous, ToneMapping toneMapping)
{
    if (toneMapping == ToneMapping::DolbyVision) {
        return appendDolbyVisionFilter(previous);
    }
    if (toneMapping == ToneMapping::Cpu) {
        // Linearize and tone-map in floating point before quantizing to SDR sRGB.
        // tonemap uses content-light/mastering metadata, or its 1000-nit fallback.
        previous = appendFilter(previous, "zscale", "thumb_linear", "transfer=linear:npl=100");
        previous = appendFilter(previous, "format", "thumb_float", "pix_fmts=gbrpf32le");
        previous = appendFilter(previous, "zscale", "thumb_primaries", "primaries=bt709");
        previous = appendFilter(previous, "tonemap", "thumb_tonemap", "tonemap=mobius:desat=0");
        previous = appendFilter(previous, "zscale", "thumb_srgb", "transfer=iec61966-2-1:primaries=bt709:matrix=gbr:range=full:dither=error_diffusion");
        previous = appendFilter(previous, "format", "thumb_planar", "pix_fmts=gbrp");
    }
    return previous;
}

std::string MovieDecoder::createDolbyVisionSourceArgs() const
{
    const auto frameRate = av_guess_frame_rate(m_pFormatContext, m_pVideoStream, nullptr);
    if (frameRate.num <= 0 || frameRate.den <= 0) {
        return {};
    }
    return ":frame_rate=" + std::to_string(frameRate.num) + "/" + std::to_string(frameRate.den);
}

AVFilterContext* MovieDecoder::appendDolbyVisionFilter(AVFilterContext* previous)
{
#if LIBAVFILTER_VERSION_INT >= AV_VERSION_INT(9, 12, 100)
    // Keep the original YUV samples and DoVi side data intact until reshaping.
    return appendFilter(previous, "libplacebo", "thumb_dovi",
                        "apply_dolbyvision=1:colorspace=gbr:color_primaries=bt709:color_trc=iec61966-2-1:range=full:format=gbrp:peak_detect=0");
#else
    throw logic_error("Dolby Vision tone mapping requires FFmpeg 6.1 or newer with libplacebo support");
#endif
}

int MovieDecoder::getWidth() const
{
    if (m_pVideoCodecContext) {
        return m_pVideoCodecContext->width;
    }

    return -1;
}

int MovieDecoder::getHeight() const
{
    if (m_pVideoCodecContext) {
        return m_pVideoCodecContext->height;
    }

    return -1;
}

int MovieDecoder::getDuration() const
{
    if (m_pFormatContext) {
        return static_cast<int>(m_pFormatContext->duration / AV_TIME_BASE);
    }

    return 0;
}

void MovieDecoder::seek(int timeInSeconds, bool accurate)
{
    if (!m_AllowSeek) {
        return;
    }

    int64_t timestamp = AV_TIME_BASE * static_cast<int64_t>(timeInSeconds);

    if (timestamp < 0) {
        timestamp = 0;
    }

    if (m_pFormatContext->start_time != AV_NOPTS_VALUE) {
        timestamp += m_pFormatContext->start_time;
    }

    // Accurate seeking needs an earlier keyframe; fast seeking keeps the next-keyframe behavior.
    const int64_t seekTimestamp = av_rescale_q(timestamp, AV_TIME_BASE_Q, m_pVideoStream->time_base);
    const int flags = accurate ? AVSEEK_FLAG_BACKWARD : 0;
    checkRc(av_seek_frame(m_pFormatContext, m_VideoStream, seekTimestamp, flags), "Seeking in video failed");
    avcodec_flush_buffers(m_pVideoCodecContext);

    if (!accurate) {
        do {
            decodeVideoFrame();
        } while (!(m_pFrame->flags & AV_FRAME_FLAG_KEY));
        return;
    }

    do {
        decodeVideoFrame();
    } while (m_pFrame->best_effort_timestamp != AV_NOPTS_VALUE &&
             av_compare_ts(m_pFrame->best_effort_timestamp, m_pVideoStream->time_base, timestamp, AV_TIME_BASE_Q) < 0);
}

void MovieDecoder::decodeVideoFrame()
{
    while (true) {
        const int rc = avcodec_receive_frame(m_pVideoCodecContext, m_pFrame);
        if (rc == 0) {
            return;
        }

        if (rc != AVERROR(EAGAIN)) {
            checkRc(rc, "Failed to decode video frame: avcodec_receive_frame()");
        }

        // Receive all available frames before sending more input. A null packet at EOF
        // drains delayed frames, including the final B-frames needed for accurate seeking.
        const bool gotPacket = getVideoPacket();
        checkRc(avcodec_send_packet(m_pVideoCodecContext, gotPacket ? m_pPacket : nullptr),
                "Failed to decode video frame: avcodec_send_packet()");
    }
}

bool MovieDecoder::getVideoPacket()
{
    bool framesAvailable = true;
    bool frameDecoded    = false;

    if (m_pPacket) {
        av_packet_unref(m_pPacket);
        delete m_pPacket;
    }

    m_pPacket = new AVPacket();

    while (framesAvailable && !frameDecoded) {
        framesAvailable = av_read_frame(m_pFormatContext, m_pPacket) >= 0;
        if (framesAvailable) {
            frameDecoded = m_pPacket->stream_index == m_VideoStream;
            if (!frameDecoded) {
                av_packet_unref(m_pPacket);
            }
        }
    }

    return frameDecoded;
}

void MovieDecoder::getScaledVideoFrame(const std::string& scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame)
{
    // Prefer per-frame tags, which can differ from container/codec defaults.
    if (m_pFrame->color_trc == AVCOL_TRC_UNSPECIFIED) {
        m_pFrame->color_trc = m_pVideoCodecContext->color_trc;
    }
    if (m_pFrame->color_primaries == AVCOL_PRI_UNSPECIFIED) {
        m_pFrame->color_primaries = m_pVideoCodecContext->color_primaries;
    }
    if (m_pFrame->colorspace == AVCOL_SPC_UNSPECIFIED) {
        m_pFrame->colorspace = m_pVideoCodecContext->colorspace;
    }
    if (m_pFrame->color_range == AVCOL_RANGE_UNSPECIFIED) {
        m_pFrame->color_range = m_pVideoCodecContext->color_range;
    }

    if (tryFilterDolbyVisionFrame(scaledSize, maintainAspectRatio, videoFrame)) {
        return;
    }

    const auto transfer = m_pFrame->color_trc;
    const auto baseMapping = (transfer == AVCOL_TRC_SMPTE2084 || transfer == AVCOL_TRC_ARIB_STD_B67)
                                 ? ToneMapping::Cpu : ToneMapping::None;
    filterVideoFrame(scaledSize, maintainAspectRatio, videoFrame, baseMapping);
}

bool MovieDecoder::tryFilterDolbyVisionFrame(const std::string& scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame)
{
    const bool hasDoviMetadata = av_frame_get_side_data(m_pFrame, AV_FRAME_DATA_DOVI_METADATA) != nullptr;
    const auto* sideData = av_packet_side_data_get(m_pVideoStream->codecpar->coded_side_data,
                                                 m_pVideoStream->codecpar->nb_coded_side_data, AV_PKT_DATA_DOVI_CONF);
    const auto* dovi = sideData && sideData->size >= sizeof(AVDOVIDecoderConfigurationRecord)
                           ? reinterpret_cast<const AVDOVIDecoderConfigurationRecord*>(sideData->data) : nullptr;
    // Profile 5 is not ordinary YUV/PQ. Only explicitly compatible base layers
    // may fall back to generic HDR/SDR processing if DoVi rendering is unavailable.
    const bool requiresDovi = (dovi && (dovi->dv_profile == 5 || dovi->dv_bl_signal_compatibility_id == 0)) ||
                              (hasDoviMetadata && !dovi);
    if (requiresDovi && !hasDoviMetadata) {
        throw logic_error("Dolby Vision input has no decoded Dolby Vision metadata; cannot render its colors correctly");
    }

    if (hasDoviMetadata && !m_DolbyVisionFilterFailed) {
        try {
            filterVideoFrame(scaledSize, maintainAspectRatio, videoFrame, ToneMapping::DolbyVision);
            return true;
        } catch (const std::logic_error& e) {
            m_DolbyVisionFilterFailed = true;
            if (requiresDovi) {
                throw logic_error(std::string("Dolby Vision tone mapping failed (FFmpeg libplacebo and a usable Vulkan device are required): ") + e.what());
            }
            av_log(nullptr, AV_LOG_WARNING, "Dolby Vision tone mapping failed; using compatible base layer: %s\n", e.what());
        }
    } else if (requiresDovi) {
        throw logic_error("Dolby Vision tone mapping is unavailable; cannot render this input's colors correctly");
    }

    return false;
}

void MovieDecoder::drainDolbyVisionFilter()
{
    // libplacebo may queue the selected still. Drain it rather than decoding
    // ahead, so a failed GPU render can retry the exact same base-layer frame.
    const auto end = m_pFrame->pts == AV_NOPTS_VALUE ? AV_NOPTS_VALUE : m_pFrame->pts + FFMAX(m_pFrame->duration, 1);
    checkRc(av_buffersrc_close(m_pFilterSource, end, 0), "Failed to drain Dolby Vision filter");
}

void MovieDecoder::filterVideoFrame(const std::string& scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame, ToneMapping toneMapping)
{
    initializeFilterGraph(m_pFormatContext->streams[m_VideoStream]->time_base, scaledSize, maintainAspectRatio, toneMapping);

    auto del = [](AVFrame* f) { av_frame_free(&f); };
    std::unique_ptr<AVFrame, decltype(del)> res(av_frame_alloc(), del);

    checkRc(av_buffersrc_write_frame(m_pFilterSource, m_pFrame), "Failed to write frame to filter graph");
    if (toneMapping == ToneMapping::DolbyVision) {
        drainDolbyVisionFilter();
    }

    int attempts = 0;
    int rc       = av_buffersink_get_frame(m_pFilterSink, res.get());
    while (rc == AVERROR(EAGAIN) && toneMapping != ToneMapping::DolbyVision && attempts++ < 10) {
        decodeVideoFrame();
        checkRc(av_buffersrc_write_frame(m_pFilterSource, m_pFrame), "Failed to write frame to filter graph");
        rc = av_buffersink_get_frame(m_pFilterSink, res.get());
    }

    checkRc(rc, "Failed to get buffer from filter");

    videoFrame.width       = res->width;
    videoFrame.height      = res->height;
    videoFrame.lineSize    = res->linesize[0];
    videoFrame.imageSource = m_UseEmbeddedData ? ThumbnailerImageSourceMetadata : ThumbnailerImageSourceVideoStream;

    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((videoFrame.frameData.data()), res->data[0], videoFrame.frameData.size());

    if (m_pFilterGraph) {
        avfilter_graph_free(&m_pFilterGraph);
    }
}

void MovieDecoder::checkRc(int ret, const std::string& message) const
{
    if (ret < 0) {
        char buf[256];
        buf[0] = ' ';
        av_strerror(ret, &buf[1], sizeof(buf) - 1);
        throw std::logic_error(message + buf);
    }
}

int32_t MovieDecoder::getStreamRotation() const
{
    if (!m_pVideoStream || !m_pVideoStream->codecpar) {
        return -1;
    }

    // For FFmpeg 5.0+
    const AVPacketSideData* side_data = av_packet_side_data_get(
        m_pVideoStream->codecpar->coded_side_data,
        m_pVideoStream->codecpar->nb_coded_side_data,
        AV_PKT_DATA_DISPLAYMATRIX);

    if (side_data && side_data->size >= sizeof(int32_t) * 9) {
        const int32_t* matrix = reinterpret_cast<const int32_t*>(side_data->data);
        auto angle            = lround(av_display_rotation_get(matrix));
        if (angle < -135) {
            return 3;
        } else if (angle > 45 && angle < 135) {
            return 2;
        } else if (angle < -45 && angle > -135) {
            return 1;
        }
    }

    return -1;
}

} // namespace ffmpegthumbnailer
