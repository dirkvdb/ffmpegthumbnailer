#ifndef PNG_WRITER_H
#define PNG_WRITER_H

#include <string>
#include <png.h>

class VideoFrame;

typedef unsigned char byte;

struct RGBData
{
	int width;
	int height;
	
	byte** imageData;
};

class PngWriter
{
public:
	PngWriter(const std::string& outputFile);
	~PngWriter();
	
	void setPngText(const std::string& key, const std::string& value);
	void writeFrame(png_byte** rgbData, int width, int height);
	
private:
	static void writeRowCallback(png_structp, png_uint_32, int);
	
private:
	FILE* 		m_FilePtr;
	png_structp m_PngPtr;
	png_infop 	m_InfoPtr;
};

#endif
