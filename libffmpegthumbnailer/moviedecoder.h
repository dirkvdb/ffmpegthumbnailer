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

#ifndef MOVIEDECODER_H
#define MOVIEDECODER_H

#include <cinttypes>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <functional>

#include "ffmpegthumbnailertypes.h"

struct AVFilterGraph;
struct AVFilterContext;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVStream;
struct AVFrame;
struct AVPacket;
struct AVRational;

namespace ffmpegthumbnailer
{

struct VideoFrame;

class MovieDecoder
{
public:
    MovieDecoder(AVFormatContext* pavContext = nullptr);
    ~MovieDecoder();

    std::string getCodec();
    void seek(int timeInSeconds);
    void decodeVideoFrame();
    void getScaledVideoFrame(const std::string& scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame);

    int getWidth();
    int getHeight();
    int getDuration();

    void initialize(const std::string& filename, bool preferEmbeddedMetadata);
    void destroy();

    bool embeddedMetaDataIsAvailable();

private:
    int32_t findPreferedVideoStream(bool preferEmbeddedMetadata);

    void initializeVideo(bool preferEmbeddedMetadata);
    void initializeFilterGraph(const AVRational& timeBase, const std::string& size, bool maintainAspectRatio);

    bool decodeVideoPacket();
    bool getVideoPacket();
    int32_t getStreamRotation();
    std::string createScaleString(const std::string& size, bool maintainAspectRatio);

    void checkRc(int ret, const std::string& message);

private:
    int                     m_VideoStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pVideoCodecContext;
    const AVCodec*          m_pVideoCodec;
    AVFilterGraph*          m_pFilterGraph;
    AVFilterContext*        m_pFilterSource;
    AVFilterContext*        m_pFilterSink;
    AVStream*               m_pVideoStream;
    AVFrame*                m_pFrame;
    AVPacket*               m_pPacket;
    bool                    m_FormatContextWasGiven;
    bool                    m_AllowSeek;
    bool                    m_UseEmbeddedData;
};

}

#endif

