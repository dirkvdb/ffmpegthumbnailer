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

#ifndef VIDEO_THUMBNAILER_H
#define VIDEO_THUMBNAILER_H

#include <string>
#include <vector>
#include <map>
#include <inttypes.h>

#include "moviedecoder.h"

typedef std::map<uint8_t, int> Histogram;

class VideoFrame;
class PngWriter;

class VideoThumbnailer
{
public:
	VideoThumbnailer(const std::string& videoFile);
	~VideoThumbnailer();
	void generateThumbnail(const std::string& outputFile, int thumbnailSize, bool filmStripOverlay, unsigned short seekPercentage, bool workaroundIssues);
    void generateThumbnail(std::vector<uint8_t>& buffer, int thumbnailSize, bool filmStripOverlay, unsigned short seekPercentage, bool workaroundIssues);
	
private:
    void generateThumbnail(PngWriter& pngWriter, int thumbnailSize, bool filmStripOverlay, unsigned short seekPercentage, bool workaroundIssues);
    void writePng(PngWriter& pngWriter, const VideoFrame& videoFrame, std::vector<uint8_t*>& rowPointers);
	
    std::string getMimeType();
	std::string getExtension(const std::string& videoFilename);

	void generateHistogram(const VideoFrame& videoFrame, Histogram& histogram);
	bool isDarkImage(const int numPixels, const Histogram& histogram);
	void overlayFilmStrip(VideoFrame& videoFrame);

private:
	MovieDecoder 	m_MovieDecoder;
	std::string		m_VideoFileName;
};

#endif
