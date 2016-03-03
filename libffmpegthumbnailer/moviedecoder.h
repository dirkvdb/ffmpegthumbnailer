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

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

struct AVFilterGraph;
struct AVFilterContext;

namespace ffmpegthumbnailer
{

struct VideoFrame;

class MovieDecoder
{
public:
    MovieDecoder(const std::string& filename, AVFormatContext* pavContext = NULL);
    ~MovieDecoder();

    std::string getCodec();
    void seek(int timeInSeconds);
    void decodeVideoFrame();
    void getScaledVideoFrame(int scaledSize, bool maintainAspectRatio, VideoFrame& videoFrame);

    int getWidth();
    int getHeight();
    int getDuration();

    void initialize(const std::string& filename);
    void destroy();

private:
    void initializeVideo();
    void initializeFilterGraph(AVRational timeBase, int width, int height);

    bool decodeVideoPacket();
    bool getVideoPacket();
    void calculateDimensions(int squareSize, bool maintainAspectRatio, int& destWidth, int& destHeight);

    void addLogCallback();
    void checkRc(int ret, const std::string& message);

private:
    int                     m_VideoStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pVideoCodecContext;
    AVCodec*                m_pVideoCodec;
    AVFilterGraph*          m_pFilterGraph;
    AVFilterContext*        m_pFilterSource;
    AVFilterContext*        m_pFilterSink;
    AVStream*               m_pVideoStream;
    AVFrame*                m_pFrame;
    AVPacket*               m_pPacket;
    bool                    m_FormatContextWasGiven;
    bool                    m_AllowSeek;
};

}

#endif

