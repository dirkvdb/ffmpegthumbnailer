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

#include "videothumbnailer.h"

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
#include <assert.h>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

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

void VideoThumbnailer::generateThumbnail(const string& videoFile, ImageWriter& imageWriter, AVFormatContext* pavContext)
{
    MovieDecoder movieDecoder(videoFile, pavContext);

    VideoFrame  videoFrame;
    movieDecoder.decodeVideoFrame(); //before seeking, a frame has to be decoded

    if ((!m_WorkAroundIssues) || (movieDecoder.getCodec() != "h264")) //workaround for bug in older ffmpeg (100% cpu usage when seeking in h264 files)
    {
        try
        {
            int secondToSeekTo = m_SeekTime.empty() ?
                movieDecoder.getDuration() * m_SeekPercentage / 100 :
                timeToSeconds(m_SeekTime);
            movieDecoder.seek(secondToSeekTo);
        }
        catch (exception& e)
        {
            cerr << e.what() << endl;
        }
    }
    
    if (m_SmartFrameSelection)
    {
        generateSmartThumbnail(movieDecoder, videoFrame);
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

void VideoThumbnailer::generateThumbnail(const string& videoFile, ImageType type, const string& outputFile, AVFormatContext* pavContext)
{
    ImageWriter* imageWriter = ImageWriterFactory<const string&>::createImageWriter(type, outputFile);
    generateThumbnail(videoFile, *imageWriter, pavContext);
    delete imageWriter;
}

void VideoThumbnailer::generateThumbnail(const string& videoFile, ImageType type, vector<uint8_t>& buffer, AVFormatContext* pavContext)
{
    buffer.clear();
    ImageWriter* imageWriter = ImageWriterFactory<vector<uint8_t>&>::createImageWriter(type, buffer);
    generateThumbnail(videoFile, *imageWriter, pavContext);
    delete imageWriter;
}

void VideoThumbnailer::writeImage(const string& videoFile, ImageWriter& imageWriter, const VideoFrame& videoFrame, int duration, vector<uint8_t*>& rowPointers)
{
    if (videoFile != "-")
    {
        struct stat statInfo;
        if (stat(videoFile.c_str(), &statInfo) == 0)
        {
            imageWriter.setText("Thumb::MTime", StringOperations::toString(statInfo.st_mtime));
            imageWriter.setText("Thumb::Size", StringOperations::toString(statInfo.st_size));
        }
        else
        {
            throw logic_error("Could not stat file: " + videoFile);
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

void VideoThumbnailer::addFilter(IFilter* filter)
{
    m_Filters.push_back(filter);
}

void VideoThumbnailer::removeFilter(IFilter* filter)
{
    for (vector<IFilter*>::iterator iter = m_Filters.begin();
         iter != m_Filters.end();
         ++iter)
    {
        if (*iter == filter)
        {
            m_Filters.erase(iter);
            break;
        }
    }
}

void VideoThumbnailer::clearFilters()
{
    m_Filters.clear();
}

void VideoThumbnailer::applyFilters(VideoFrame& frameData)
{
    for (vector<IFilter*>::iterator iter = m_Filters.begin();
         iter != m_Filters.end();
         ++iter)
    {
        (*iter)->process(frameData);
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
