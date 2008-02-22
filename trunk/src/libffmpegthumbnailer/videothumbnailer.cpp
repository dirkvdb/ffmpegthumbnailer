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

#include "videothumbnailer.hpp"

#include "pngwriter.hpp"
#include "moviedecoder.hpp"
#include "stringoperations.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

static const int FILMHOLE_WIDTH = 12;
static const int FILMHOLE_HEIGHT = 10;
	
static const uint8_t filmHole[FILMHOLE_WIDTH * FILMHOLE_HEIGHT * 3] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x23, 0x23, 0x7a, 0x7a, 0x7a, 0x83, 0x83, 0x83, 0x8c, 0x8c, 0x8c, 0x90, 0x90, 0x90, 0x8e, 0x8e, 0x8e, 0x52, 0x52, 0x52, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x6e, 0x6e, 0x83, 0x83, 0x83, 0x93, 0x93, 0x93, 0xa3, 0xa3, 0xa3, 0xab, 0xab, 0xab, 0xa8, 0xa8, 0xa8, 0x9b, 0x9b, 0x9b, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x72, 0x72, 0x8e, 0x8e, 0x8e, 0xa4, 0xa4, 0xa4, 0xbb, 0xbb, 0xbb, 0xc4, 0xc4, 0xc4, 0xc1, 0xc1, 0xc1, 0xaf, 0xaf, 0xaf, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e, 0x90, 0x90, 0x90, 0xa6, 0xa6, 0xa6, 0xbe, 0xbe, 0xbe, 0xc8, 0xc8, 0xc8, 0xc4, 0xc4, 0xc4, 0x8e, 0x8e, 0x8e, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


VideoThumbnailer::VideoThumbnailer()
: m_ThumbnailSize(128)
, m_SeekPercentage(10)
, m_OverlayFilmStrip(false)
, m_WorkAroundIssues(false)
{
}

VideoThumbnailer::VideoThumbnailer(int thumbnailSize, uint16_t seekPercentage, bool filmStripOverlay, bool workaroundIssues)
: m_ThumbnailSize(thumbnailSize)
, m_SeekPercentage(seekPercentage)
, m_OverlayFilmStrip(filmStripOverlay)
, m_WorkAroundIssues(workaroundIssues)
{
}

VideoThumbnailer::~VideoThumbnailer()
{
}

void VideoThumbnailer::setSeekPercentage(int percentage)
{
    m_SeekPercentage = percentage > 95 ? 95 : percentage;
}

void VideoThumbnailer::setThumbnailSize(int size)
{
    m_ThumbnailSize = size;
}

void VideoThumbnailer::setWorkAroundIssues(bool workAround)
{
    m_WorkAroundIssues = workAround;
}

void VideoThumbnailer::setFilmStripOverlay(bool enabled)
{
    m_OverlayFilmStrip = enabled;
}

void VideoThumbnailer::generateThumbnail(const string& videoFile, PngWriter& pngWriter)
{
    MovieDecoder movieDecoder(videoFile);
    
	VideoFrame 	videoFrame;
	movieDecoder.decodeVideoFrame(); //before seeking, a frame has to be decoded
    
	if ((!m_WorkAroundIssues) || (movieDecoder.getCodec() != "h264")) //workaround for bug in older ffmpeg (100% cpu usage when seeking in h264 files)
	{
		try
		{
			movieDecoder.seek(movieDecoder.getDuration() * m_SeekPercentage / 100);
		}
		catch (exception& e)
		{
			cerr << e.what() << endl;
		}
	}
    
	movieDecoder.getScaledVideoFrame(m_ThumbnailSize, videoFrame);
	if (m_OverlayFilmStrip && (videoFrame.width > FILMHOLE_WIDTH * 2))
	{
		overlayFilmStrip(videoFrame);
	}
    
	vector<uint8_t*> rowPointers;
	for (int i = 0; i < videoFrame.height; ++i)
	{
		rowPointers.push_back(&(videoFrame.frameData[i * videoFrame.lineSize])); 
	}
	
	writePng(videoFile, pngWriter, videoFrame, movieDecoder.getDuration(), rowPointers);
}

void VideoThumbnailer::generateThumbnail(const string& videoFile, const string& outputFile)
{
    PngWriter pngWriter(outputFile);
    generateThumbnail(videoFile, pngWriter);
}

void VideoThumbnailer::generateThumbnail(const std::string& videoFile, std::vector<uint8_t>& buffer)
{
    buffer.clear();
    PngWriter pngWriter(buffer);
    generateThumbnail(videoFile, pngWriter);
}

void VideoThumbnailer::writePng(const string& videoFile, PngWriter& pngWriter, const VideoFrame& videoFrame, int duration, vector<uint8_t*>& rowPointers)
{
    struct stat statInfo;
    if (stat(videoFile.c_str(), &statInfo) == 0)
    {
		pngWriter.setPngText("Thumb::MTime", StringOperations::toString(statInfo.st_mtime));
		pngWriter.setPngText("Thumb::Size", StringOperations::toString(statInfo.st_size));
    }
    else
    {
    	throw logic_error("Could not stat file: " + videoFile);
    } 
    
    string mimeType = getMimeType(videoFile);
    if (!mimeType.empty())
    {
    	pngWriter.setPngText("Thumb::Mimetype", mimeType);
    }
	
	pngWriter.setPngText("Thumb::URI", videoFile);
	pngWriter.setPngText("Thumb::Movie::Length", StringOperations::toString(duration));
    pngWriter.writeFrame(&(rowPointers.front()), videoFrame.width, videoFrame.height);
}

string VideoThumbnailer::getMimeType(const string& videoFile)
{
	string extension = getExtension(videoFile);
	
	if (extension == "avi")
	{
		return "video/x-msvideo";
	}
	else if (extension == "mpeg" || extension == "mpg" || extension == "mpe" || extension == "vob")
	{
		return "video/mpeg";
	}
	else if (extension == "qt" || extension == "mov")
	{
		return "video/quicktime";
	}
	else if (extension == "asf" || extension == "asx")
	{
		return "video/x-ms-asf";
	}
	else if (extension == "wm")
	{
		return "video/x-ms-wm";
	}
	else if (extension == "mp4")
	{
		return "video/x-ms-wmv";
	}
	else if (extension == "mp4")
	{
		return "video/mp4";
	}
	else if (extension == "flv")
	{
		return "video/x-flv";
	}
	else
	{
		return "";
	}
}

string VideoThumbnailer::getExtension(const string& videoFilename)
{
	string extension;
	string::size_type pos = videoFilename.rfind('.');
	
	if (string::npos != pos)
	{
		extension = videoFilename.substr(pos + 1, videoFilename.size());
	}
	
	return extension;
}

void VideoThumbnailer::generateHistogram(const VideoFrame& videoFrame, std::map<uint8_t, int>& histogram)
{
	for (int i = 0; i < videoFrame.height; ++i)
	{
		int pixelIndex = i * videoFrame.lineSize;
		for (int j = 0; j < videoFrame.width * 3; j += 3)
		{
			uint8_t r = videoFrame.frameData[pixelIndex + j];
			uint8_t g = videoFrame.frameData[pixelIndex + j + 1];
			uint8_t b = videoFrame.frameData[pixelIndex + j + 2];
			
			uint8_t luminance = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
			histogram[luminance] += 1;
		}
	}
}

bool VideoThumbnailer::isDarkImage(const int numPixels, const Histogram& histogram)
{
	int darkPixels = 0;
	
	Histogram::const_iterator iter;
	for(iter = histogram.begin(); iter->first < 15; ++iter)
	{
		darkPixels += iter->second;
	}
	
	return darkPixels > static_cast<int>(numPixels / 2);
}

void VideoThumbnailer::overlayFilmStrip(VideoFrame& videoFrame)
{
	int frameIndex = 0;
	int filmHoleIndex = 0;
	int offset = (videoFrame.width * 3) - 3;
	
	for (int i = 0; i < videoFrame.height; ++i)
	{
		for (int j = 0; j < FILMHOLE_WIDTH * 3; j+=3)
		{
			int currentFilmHoleIndex = filmHoleIndex + j;
						
			videoFrame.frameData[frameIndex + j]     = filmHole[currentFilmHoleIndex];
			videoFrame.frameData[frameIndex + j + 1] = filmHole[currentFilmHoleIndex + 1];
			videoFrame.frameData[frameIndex + j + 2] = filmHole[currentFilmHoleIndex + 2];
			
			videoFrame.frameData[frameIndex + offset - j]     = filmHole[currentFilmHoleIndex];
			videoFrame.frameData[frameIndex + offset - j + 1] = filmHole[currentFilmHoleIndex + 1];
			videoFrame.frameData[frameIndex + offset - j + 2] = filmHole[currentFilmHoleIndex + 2];
		}
		frameIndex += videoFrame.lineSize;
		filmHoleIndex = (i % FILMHOLE_HEIGHT) * FILMHOLE_WIDTH * 3;
	}
}
