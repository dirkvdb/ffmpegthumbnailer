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

#include "moviedecoder.h"
#include <stdexcept>

#include <assert.h>
#include <iostream>
#include <cstring>

extern "C" {
#if HAVE_LIBSWSCALE_SWSCALE_H
#include <libswscale/swscale.h>
#else
#include <ffmpeg/swscale.h>
#endif
}

using namespace std;

MovieDecoder::MovieDecoder(const string& filename, AVFormatContext* pavContext)
: m_VideoStream(-1)
, m_pFormatContext(pavContext)
, m_pVideoCodecContext(NULL)
, m_pVideoCodec(NULL)
, m_pVideoStream(NULL)
, m_pFrame(NULL)
, m_pPacket(NULL)
, m_FormatContextWasGiven(pavContext != NULL)
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
    avcodec_init();
    avcodec_register_all();

    if ((!m_FormatContextWasGiven) && av_open_input_file(&m_pFormatContext, filename.c_str(), NULL, 0, NULL) != 0)
    {
        throw logic_error(string("Could not open input file: ") + filename);
    }

    if (av_find_stream_info(m_pFormatContext) < 0)
    {
        throw logic_error(string("Could not find stream information"));
    }

    initializeVideo();
    m_pFrame = avcodec_alloc_frame();
}

void MovieDecoder::destroy()
{
    if (m_pVideoCodecContext)
    {
        avcodec_close(m_pVideoCodecContext);
        m_pVideoCodecContext = NULL;
    }

    if ((!m_FormatContextWasGiven) && m_pFormatContext)
    {
        av_close_input_file(m_pFormatContext);
        m_pFormatContext = NULL;
    }
    
    if (m_pPacket)
    {
        av_free_packet(m_pPacket);
	delete m_pPacket;
        m_pPacket = NULL;
    }
    
    if (m_pFrame)
    {
        av_free(m_pFrame);
        m_pFrame = NULL;
    }
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
    for(unsigned int i = 0; i < m_pFormatContext->nb_streams; i++)
    {
        if (m_pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO)
        {
            m_pVideoStream = m_pFormatContext->streams[i];
            m_VideoStream = i;
            break;
        }
    }

    if (m_VideoStream == -1)
    {
        throw logic_error("Could not find video stream");
    }

    m_pVideoCodecContext = m_pFormatContext->streams[m_VideoStream]->codec;
    m_pVideoCodec = avcodec_find_decoder(m_pVideoCodecContext->codec_id);

    if (m_pVideoCodec == NULL)
    {
        // set to NULL, otherwise avcodec_close(m_pVideoCodecContext) crashes
        m_pVideoCodecContext = NULL;
        throw logic_error("Video Codec not found");
    }

    m_pVideoCodecContext->workaround_bugs = 1;
    m_pFormatContext->flags |= AVFMT_FLAG_GENPTS;

    if (avcodec_open(m_pVideoCodecContext, m_pVideoCodec) < 0)
    {
        throw logic_error("Could not open video codec");
    }
}

int MovieDecoder::getWidth()
{
    if (m_pVideoCodecContext)
    {
        return m_pVideoCodecContext->height;
    }
    
    return -1;
}

int MovieDecoder::getHeight()
{
    if (m_pVideoCodecContext)
    {
        return m_pVideoCodecContext->width;
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
    int64_t timestamp = (::int64_t) (AV_TIME_BASE * timeInSeconds);

    if (timestamp < 0)
    {
        timestamp = 0;
    }
   
    int ret = av_seek_frame(m_pFormatContext, -1, timestamp, 0);
    if (ret >= 0)
    {
        avcodec_flush_buffers(m_pFormatContext->streams[m_VideoStream]->codec);
    }
    else
    {
        throw logic_error("Seeking in video failed");
    }
    
    int count = 0;
    do
    {
        bool gotFrame = 0;

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

        ++count;
    } while ((!m_pFrame->key_frame) && count < 20);
}


void MovieDecoder::decodeVideoFrame()
{
    bool        frameFinished = false;
    
    while(!frameFinished && getVideoPacket())
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
    assert(m_pPacket->stream_index == m_VideoStream);
    int frameFinished;
    
    int bytesDecoded = avcodec_decode_video(m_pVideoCodecContext, m_pFrame, &frameFinished, 
                                        m_pPacket->data, m_pPacket->size);
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
        av_free_packet(m_pPacket);
        delete m_pPacket;
    }
    
    m_pPacket = new AVPacket();
    
    while (framesAvailable && !frameDecoded && attempts++ < 250)
    {
        framesAvailable = av_read_frame(m_pFormatContext, m_pPacket) >= 0;
        
        if (framesAvailable)
        {
            frameDecoded = m_pPacket->stream_index == m_VideoStream;
            if (!frameDecoded)
            {
                av_free_packet(m_pPacket);
            }
        }
    }
    
    return frameDecoded;
}

void MovieDecoder::getScaledVideoFrame(int scaledSize, VideoFrame& videoFrame)
{
    if (m_pFrame->interlaced_frame)
    {
        avpicture_deinterlace((AVPicture*) m_pFrame, (AVPicture*) m_pFrame, m_pVideoCodecContext->pix_fmt,
                              m_pVideoCodecContext->width, m_pVideoCodecContext->height);
    }
    
    int scaledWidth, scaledHeight;
    convertAndScaleFrame(PIX_FMT_RGB24, scaledSize, scaledWidth, scaledHeight);
    
    videoFrame.width = scaledWidth;
    videoFrame.height = scaledHeight;
    videoFrame.lineSize = m_pFrame->linesize[0];
    
    videoFrame.frameData.clear();
    videoFrame.frameData.resize(videoFrame.lineSize * videoFrame.height);
    memcpy((&(videoFrame.frameData.front())), m_pFrame->data[0], videoFrame.lineSize * videoFrame.height);
}

void MovieDecoder::convertAndScaleFrame(int format, int scaledSize, int& scaledWidth, int& scaledHeight)
{
    calculateDimensions(m_pVideoCodecContext->width, m_pVideoCodecContext->height, scaledSize, scaledWidth, scaledHeight);
    SwsContext* scaleContext = sws_getContext(m_pVideoCodecContext->width, m_pVideoCodecContext->height,
                                              m_pVideoCodecContext->pix_fmt, scaledWidth, scaledHeight,
                                              format, SWS_BICUBIC, NULL, NULL, NULL);

    if (NULL == scaleContext)
    {
        throw logic_error("Failed to create resize context");
    }
    
    AVFrame* convertedFrame = NULL;
    
    createAVFrame(&convertedFrame, scaledWidth, scaledHeight, format);

    sws_scale(scaleContext, m_pFrame->data, m_pFrame->linesize, 0, m_pVideoCodecContext->height,
              convertedFrame->data, convertedFrame->linesize);
    sws_freeContext(scaleContext);

    av_free(m_pFrame);
    m_pFrame = convertedFrame;
}

void MovieDecoder::calculateDimensions(int srcWidth, int srcHeight, int squareSize, int& destWidth, int& destHeight)
{
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

void MovieDecoder::createAVFrame(AVFrame** avFrame, int width, int height, int format)
{
    *avFrame        = avcodec_alloc_frame();
    
    int         numBytes = avpicture_get_size(format, width, height);
    uint8_t*    pBuffer = new uint8_t[numBytes];
    
    avpicture_fill((AVPicture*) *avFrame, pBuffer, format, width, height);
}
