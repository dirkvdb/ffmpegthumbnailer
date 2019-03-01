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

#ifndef VIDEO_THUMBNAILER_H
#define VIDEO_THUMBNAILER_H

#include <string>
#include <vector>
#include <map>
#include <cinttypes>
#include <functional>

#include "ifilter.h"
#include "ffmpegthumbnailertypes.h"

struct AVFormatContext;

namespace ffmpegthumbnailer
{

struct VideoFrame;
class ImageWriter;
class MovieDecoder;
template <typename T>
struct Histogram;

struct VideoFrameInfo
{
    int width = 0;
    int height = 0;
    ThumbnailerImageSource source = ThumbnailerImageSourceVideoStream;
};

class VideoThumbnailer
{
public:
    VideoThumbnailer();
    VideoThumbnailer(int thumbnailSize, bool workaroundIssues, bool maintainAspectRatio, int imageQuality, bool smartFrameSelection);

    VideoFrameInfo generateThumbnail(const std::string& videoFile, ThumbnailerImageType type, const std::string& outputFile, AVFormatContext* pAvContext = nullptr);
    VideoFrameInfo generateThumbnail(const std::string& videoFile, ThumbnailerImageType type, std::vector<uint8_t>& buffer, AVFormatContext* pAvContext = nullptr);

    void setThumbnailSize(int size);
    void setThumbnailSize(int width, int height);
    void setThumbnailSize(const std::string& size);
    void setSeekPercentage(int percentage);
    void setSeekTime(const std::string& seekTime);
    void setWorkAroundIssues(bool workAround);
    void setImageQuality(int imageQuality);
    void setMaintainAspectRatio(bool enabled);
    void setPreferEmbeddedMetadata(bool enabled);
    void setSmartFrameSelection(bool enabled);
    void addFilter(IFilter* pFilter);
    void removeFilter(IFilter* pFilter);
    void clearFilters();

    void setLogCallback(std::function<void(ThumbnailerLogLevel, const std::string&)> cb);

private:
    VideoFrameInfo generateThumbnail(const std::string& videoFile, ImageWriter& imageWriter, AVFormatContext* pAvContext = nullptr);
    void generateSmartThumbnail(MovieDecoder& movieDecoder, VideoFrame& videoFrame);
    void writeImage(const std::string& videoFile, ImageWriter& imageWriter, const VideoFrame& videoFrame, int duration, std::vector<uint8_t*>& rowPointers);

    std::string getMimeType(const std::string& videoFile);
    std::string getExtension(const std::string& videoFilename);

    int getBestThumbnailIndex(std::vector<VideoFrame>& videoFrames, const std::vector<Histogram<int> >& histograms);
    void applyFilters(VideoFrame& frameData);

    void TraceMessage(ThumbnailerLogLevel lvl, const std::string& msg);

private:
    std::string                                     m_ThumbnailSize;
    uint16_t                                        m_SeekPercentage;
    bool                                            m_OverlayFilmStrip;
    bool                                            m_WorkAroundIssues;
    int                                             m_ImageQuality;
    bool                                            m_MaintainAspectRatio;
    bool                                            m_SmartFrameSelection;
    bool                                            m_PreferEmbeddedMetadata;
    std::string                                     m_SeekTime;
    std::vector<IFilter*>                           m_Filters;

    std::function<void(ThumbnailerLogLevel, const std::string&)> m_LogCb;
};

}

#endif
