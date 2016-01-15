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

#include "config.h"

#include "videothumbnailer.h"

#include "histogram.h"
#include "moviedecoder.h"
#include "stringoperations.h"
#include "filmstripfilter.h"
#include "imagewriterfactory.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cfloat>
#include <cmath>
#include <stdexcept>
#include <cassert>
#include <cerrno>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

namespace ffmpegthumbnailer
{

static const int SMART_FRAME_ATTEMPTS = 25;

VideoThumbnailer::VideoThumbnailer()
: m_ThumbnailSize(128)
, m_SeekPercentage(10)
, m_OverlayFilmStrip(false)
, m_WorkAroundIssues(false)
, m_ImageQuality(8)
, m_MaintainAspectRatio(true)
, m_SmartFrameSelection(false)
{
}

VideoThumbnailer::VideoThumbnailer(int thumbnailSize, bool workaroundIssues, bool maintainAspectRatio, int imageQuality, bool smartFrameSelection)
: m_ThumbnailSize(thumbnailSize)
, m_SeekPercentage(10)
, m_OverlayFilmStrip(false)
, m_WorkAroundIssues(workaroundIssues)
, m_ImageQuality(imageQuality)
, m_MaintainAspectRatio(maintainAspectRatio)
, m_SmartFrameSelection(smartFrameSelection)
{
}

VideoThumbnailer::~VideoThumbnailer()
{
}

void VideoThumbnailer::setSeekPercentage(int percentage)
{
    m_SeekTime.clear();
    m_SeekPercentage = percentage > 95 ? 95 : percentage;
}

void VideoThumbnailer::setSeekTime(const std::string& seekTime)
{
    m_SeekTime = seekTime;
}

void VideoThumbnailer::setThumbnailSize(int size)
{
    m_ThumbnailSize = size;
}

void VideoThumbnailer::setWorkAroundIssues(bool workAround)
{
    m_WorkAroundIssues = workAround;
}

void VideoThumbnailer::setImageQuality(int imageQuality)
{
    m_ImageQuality = imageQuality;
}

void VideoThumbnailer::setMaintainAspectRatio(bool enabled)
{
    m_MaintainAspectRatio = enabled;
}

void VideoThumbnailer::setSmartFrameSelection(bool enabled)
{
    m_SmartFrameSelection = enabled;
}

int timeToSeconds(const std::string& time)
{
    int hours, minutes, seconds;
    sscanf(time.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);

    return (hours * 3600) + (minutes * 60) + seconds;
}

void VideoThumbnailer::generateThumbnail(const string& videoFile, ImageWriter& imageWriter, AVFormatContext* pAvContext)
{
    MovieDecoder movieDecoder(videoFile, pAvContext);
    movieDecoder.decodeVideoFrame(); //before seeking, a frame has to be decoded

    if ((!m_WorkAroundIssues) || (movieDecoder.getCodec() != "h264")) //workaround for bug in older ffmpeg (100% cpu usage when seeking in h264 files)
    {
        try
        {
            int secondToSeekTo = m_SeekTime.empty() ? movieDecoder.getDuration() * m_SeekPercentage / 100
													: timeToSeconds(m_SeekTime);
            movieDecoder.seek(secondToSeekTo);
        }
        catch (const exception& e)
        {
            TraceMessage(ThumbnailerLogLevelError, std::string(e.what()) + ", will use first frame.");  
            
            //seeking failed, try the first frame again
            movieDecoder.destroy();
            movieDecoder.initialize(videoFile);
            movieDecoder.decodeVideoFrame();
         }
    }

    VideoFrame  videoFrame;

    if (m_SmartFrameSelection)
    {
        try
        {
            generateSmartThumbnail(movieDecoder, videoFrame);
        }
        catch (const exception& e)
        {
            TraceMessage(ThumbnailerLogLevelError, std::string(e.what()) + ". Smart frame selection failed. Retrying without smart frame detection.");  
            m_SmartFrameSelection = false;
            generateThumbnail(videoFile, imageWriter, pAvContext);
        }
    }
    else
    {
        movieDecoder.getScaledVideoFrame(m_ThumbnailSize, m_MaintainAspectRatio, videoFrame);
    }

    applyFilters(videoFrame);

    vector<uint8_t*> rowPointers;
    for (int i = 0; i < videoFrame.height; ++i)
    {
        rowPointers.push_back(&(videoFrame.frameData[i * videoFrame.lineSize]));
    }

    writeImage(videoFile, imageWriter, videoFrame, movieDecoder.getDuration(), rowPointers);
}

void VideoThumbnailer::generateSmartThumbnail(MovieDecoder& movieDecoder, VideoFrame& videoFrame)
{
    vector<VideoFrame> videoFrames(SMART_FRAME_ATTEMPTS);
    vector<Histogram<int> > histograms(SMART_FRAME_ATTEMPTS);

    for (int i = 0; i < SMART_FRAME_ATTEMPTS; ++i)
    {
        movieDecoder.decodeVideoFrame();
        movieDecoder.getScaledVideoFrame(m_ThumbnailSize, m_MaintainAspectRatio, videoFrames[i]);
        generateHistogram(videoFrames[i], histograms[i]);
    }

    int bestFrame = getBestThumbnailIndex(videoFrames, histograms);

    assert(bestFrame != -1);
    videoFrame = videoFrames[bestFrame];
}

void VideoThumbnailer::generateThumbnail(const string& videoFile, ThumbnailerImageType type, const string& outputFile, AVFormatContext* pAvContext)
{
    ImageWriter* imageWriter = ImageWriterFactory<const string&>::createImageWriter(type, outputFile);
    generateThumbnail(videoFile, *imageWriter, pAvContext);
    delete imageWriter;
}

void VideoThumbnailer::generateThumbnail(const string& videoFile, ThumbnailerImageType type, vector<uint8_t>& buffer, AVFormatContext* pAvContext)
{
    buffer.clear();
    ImageWriter* imageWriter = ImageWriterFactory<vector<uint8_t>&>::createImageWriter(type, buffer);
    generateThumbnail(videoFile, *imageWriter, pAvContext);
    delete imageWriter;
}

void VideoThumbnailer::writeImage(const string& videoFile, ImageWriter& imageWriter, const VideoFrame& videoFrame, int duration, vector<uint8_t*>& rowPointers)
{
    if ((videoFile != "-") &&
        (videoFile.compare(0, 7, "rtsp://") != 0) &&
        (videoFile.compare(0, 6, "udp://") != 0) &&
        (videoFile.compare(0, 7, "http://") != 0))
    {
        struct stat statInfo;
        if (stat(videoFile.c_str(), &statInfo) == 0)
        {
            imageWriter.setText("Thumb::MTime", StringOperations::toString(statInfo.st_mtime));
            imageWriter.setText("Thumb::Size", StringOperations::toString(statInfo.st_size));
        }
        else
        {
            cout << "Warn: Failed to stat file " << videoFile << " (" << strerror(errno) << ")" << endl;
        }

        string mimeType = getMimeType(videoFile);
        if (!mimeType.empty())
        {
            imageWriter.setText("Thumb::Mimetype", mimeType);
        }

        imageWriter.setText("Thumb::URI", videoFile);
        imageWriter.setText("Thumb::Movie::Length", StringOperations::toString(duration));
    }

    imageWriter.writeFrame(&(rowPointers.front()), videoFrame.width, videoFrame.height, m_ImageQuality);
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
    else if (extension == "wmv")
    {
        return "video/x-ms-wmv";
    }
    else if (extension == "mp4")
    {
        return "video/mp4";
    }
    else if (extension == "webm")
    {
        return "video/webm";
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
    auto pos = videoFilename.rfind('.');

    if (string::npos != pos)
    {
        extension = videoFilename.substr(pos + 1, videoFilename.size());
    }

    return extension;
}

void VideoThumbnailer::addFilter(IFilter* pFilter)
{
    m_Filters.push_back(pFilter);
}

void VideoThumbnailer::removeFilter(IFilter* pFilter)
{
    std::remove_if(m_Filters.begin(), m_Filters.end(), [pFilter] (IFilter* fil) {
        return fil == pFilter;
    });
}

void VideoThumbnailer::clearFilters()
{
    m_Filters.clear();
}

void VideoThumbnailer::setLogCallback(std::function<void(ThumbnailerLogLevel, const std::string&)> cb)
{
    m_LogCb = cb;
}

void VideoThumbnailer::applyFilters(VideoFrame& frameData)
{
    for (auto filter : m_Filters)
    {
        filter->process(frameData);
    }
}

void VideoThumbnailer::generateHistogram(const VideoFrame& videoFrame, Histogram<int>& histogram)
{
    for (int i = 0; i < videoFrame.height; ++i)
    {
        int pixelIndex = i * videoFrame.lineSize;
        for (int j = 0; j < videoFrame.width * 3; j += 3)
        {
            ++histogram.r[videoFrame.frameData[pixelIndex + j]];
            ++histogram.g[videoFrame.frameData[pixelIndex + j + 1]];
            ++histogram.b[videoFrame.frameData[pixelIndex + j + 2]];
        }
    }
}

int VideoThumbnailer::getBestThumbnailIndex(vector<VideoFrame>& videoFrames, const vector<Histogram<int> >& histograms)
{
    Histogram<float> avgHistogram;
    for (size_t i = 0; i < histograms.size(); ++i)
    {
        for (int j = 0; j < 255; ++j)
        {
            avgHistogram.r[j] += static_cast<float>(histograms[i].r[j]) / histograms.size();
            avgHistogram.g[j] += static_cast<float>(histograms[i].g[j]) / histograms.size();
            avgHistogram.b[j] += static_cast<float>(histograms[i].b[j]) / histograms.size();
        }
    }

    int bestFrame = -1;
    float minRMSE = FLT_MAX;
    for (size_t i = 0; i < histograms.size(); ++i)
    {
        //calculate root mean squared error
        float rmse = 0.0;
        for (int j = 0; j < 255; ++j)
        {
            float error = fabsf(avgHistogram.r[j] - histograms[i].r[j])
                        + fabsf(avgHistogram.g[j] - histograms[i].g[j])
                        + fabsf(avgHistogram.b[j] - histograms[i].b[j]);
            rmse += (error * error) / 255;
        }

        rmse = sqrtf(rmse);
        if (rmse < minRMSE)
        {
            minRMSE = rmse;
            bestFrame = i;
        }

#ifdef DEBUG_MODE
        stringstream outputFile;
        outputFile << "frames/Frame" << setfill('0') << setw(3) << i << "_" << rmse << endl;
        ImageWriter* imageWriter = ImageWriterFactory<const string&>::createImageWriter(Png, outputFile.str());
        vector<uint8_t*> rowPointers;
        for (int x = 0; x < videoFrames[i].height; ++x)
            rowPointers.push_back(&(videoFrames[i].frameData[x * videoFrames[i].lineSize]));
        imageWriter->writeFrame(&(rowPointers.front()), videoFrames[i].width, videoFrames[i].height, m_ImageQuality);
        delete imageWriter;
#endif
    }

#ifdef DEBUG_MODE
    cout << "Best frame was: " << bestFrame << "(RMSE: " << minRMSE << ")" << endl;
#endif

    return bestFrame;
}

void VideoThumbnailer::TraceMessage(ThumbnailerLogLevel lvl, const std::string& msg)
{
    if (m_LogCb)
    {
        m_LogCb(lvl, msg);
    }
}

}

