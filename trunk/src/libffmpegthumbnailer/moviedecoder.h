//    Copyright (C) 2007 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#define __STDC_CONSTANT_MACROS
#define INT64_C
#include <inttypes.h>

#include <string>
#include <vector>

extern "C" {
#if HAVE_LIBAVCODEC_AVCODEC_H
#include <libavcodec/avcodec.h>
#else
#include <ffmpeg/avcodec.h>
#endif
#if HAVE_LIBAVFORMAT_AVFORMAT_H
#include <libavformat/avformat.h>
#else
#include <ffmpeg/avformat.h>
#endif
}

struct VideoFrame
{
    int width;
    int height;
    int lineSize;
    
    std::vector<uint8_t> frameData;
};

class MovieDecoder
{
public:
    MovieDecoder(const std::string& filename, AVFormatContext* pavContext = NULL);
    ~MovieDecoder();
    
    std::string getCodec();
    void seek(int timeInSeconds);
    void decodeVideoFrame();
    void getScaledVideoFrame(int scaledSize, VideoFrame& videoFrame);

    int getWidth();
    int getHeight();    
    int getDuration();
    
private:
    void initialize(const std::string& filename);
    void initializeVideo();
    void destroy();
    
    bool decodeVideoPacket();
    bool getVideoPacket();
    void convertAndScaleFrame(int format, int scaledSize, int& scaledWidth, int& scaledHeight);
    void createAVFrame(AVFrame** avFrame, int width, int height, int format);
    void calculateDimensions(int srcWidth, int srcHeight, int squareSize, int& destWidth, int& destHeight);
    
private:
    int                     m_VideoStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pVideoCodecContext;
    AVCodec*                m_pVideoCodec;
    AVStream*               m_pVideoStream;
    AVFrame*                m_pFrame;
    AVPacket*               m_pPacket;
    bool                    m_FormatContextWasGiven;
};

#endif
