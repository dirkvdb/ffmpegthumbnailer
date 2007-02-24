#ifndef VIDEO_THUMBNAILER_H
#define VIDEO_THUMBNAILER_H

#include <string>
#include <vector>
#include <map>

#include "moviedecoder.h"

typedef unsigned char byte;
typedef std::map<byte, int> Histogram;

class VideoFrame;

class VideoThumbnailer
{
public:
	VideoThumbnailer(const std::string& videoFile);
	~VideoThumbnailer();
	void generateThumbnail(const std::string& outputFile, int thumbnailSize);
	
private:
	void writePng(const std::string& outputFile, const VideoFrame& videoFrame, std::vector<byte*>& rowPointers);
	std::string getMimeType();
	std::string getExtension(const std::string& videoFilename);

	void generateHistogram(const VideoFrame& videoFrame, Histogram& histogram);
	bool isDarkImage(const int numPixels, const Histogram& histogram);

private:
	MovieDecoder 	m_MovieDecoder;
	std::string		m_VideoFileName;
};

#endif
