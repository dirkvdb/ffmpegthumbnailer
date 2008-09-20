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

#include "imagetypes.h"
#include "imagewriterfactory.h"

class VideoFrame;
class ImageWriter;
struct AVFormatContext;

class VideoThumbnailer
{
public:
    VideoThumbnailer();
    VideoThumbnailer(int thumbnailSize, bool filmStripOverlay, bool workaroundIssues, bool maintainAspectRatio, int imageQuality);
    ~VideoThumbnailer();

    void generateThumbnail(const std::string& videoFile, ImageType type, const std::string& outputFile, AVFormatContext* pavContext = NULL);
    void generateThumbnail(const std::string& videoFile, ImageType type, std::vector<uint8_t>& buffer, AVFormatContext* pavContext = NULL);

    void setThumbnailSize(int size);
    void setSeekPercentage(int percentage);
    void setSeekTime(const std::string& seekTime);
    void setFilmStripOverlay(bool enabled);
    void setWorkAroundIssues(bool workAround);
    void setImageQuality(int imageQuality);
    void setMaintainAspectRatio(bool enabled);

private:
    typedef std::map<uint8_t, int> Histogram;

    void generateThumbnail(const std::string& videoFile, ImageWriter& imageWriter, AVFormatContext* pavContext = NULL);
    void writeImage(const std::string& videoFile, ImageWriter& imageWriter, const VideoFrame& videoFrame, int duration, std::vector<uint8_t*>& rowPointers);

    std::string getMimeType(const std::string& videoFile);
    std::string getExtension(const std::string& videoFilename);

    void generateHistogram(const VideoFrame& videoFrame, Histogram& histogram);
    bool isDarkImage(const int numPixels, const Histogram& histogram);
    void overlayFilmStrip(VideoFrame& videoFrame);

private:
    int             m_ThumbnailSize;
    uint16_t        m_SeekPercentage;
    bool            m_OverlayFilmStrip;
    bool            m_WorkAroundIssues;
    int             m_ImageQuality;
    bool            m_MaintainAspectRatio;
    std::string     m_SeekTime;
};

#endif
