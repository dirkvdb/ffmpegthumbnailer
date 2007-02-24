#ifndef MOVIEDECODER_H
#define MOVIEDECODER_H

#define EMULATE_INTTYPES

#include <string>
#include <vector>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>

typedef unsigned char byte;

struct VideoFrame
{
	int width;
	int height;
	int lineSize;
	
	std::vector<byte> frameData;
};

class MovieDecoder
{
public:
    MovieDecoder(const std::string& filename);
    ~MovieDecoder();
    
    std::string getCodec();
    void seek(int timeInSeconds);
	void decodeVideoFrame();
    //void decodeVideoFrame(VideoFrame& videoFrame, int scaledSize);
	void getScaledVideoFrame(int scaledSize, VideoFrame& videoFrame);

	int getWidth();
	int getHeight();    
    int getDuration();
    
private:
    void initialize(const std::string& filename);
    void initializeVideo();
    void destroy();
    
	bool decodeVideoPacket(AVPacket& packet);
    bool getVideoPacket(AVPacket& packet);
    void convertAndScaleFrame(int format, int scaledSize, int& scaledWidth, int& scaledHeight);
    void createAVFrame(AVFrame** avFrame, int width, int height, int format);
    void calculateDimensions(int srcWidth, int srcHeight, int squareSize, int& destWidth, int& destHeight);
    
private:
    int                     m_VideoStream;
    AVFormatContext*        m_pFormatContext;
    AVCodecContext*         m_pVideoCodecContext;
    AVCodec*                m_pVideoCodec;
    AVStream*               m_pVideoStream;
	AVFrame*				m_pFrame;
};

#endif
