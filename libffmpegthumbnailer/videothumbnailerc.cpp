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

#include "videothumbnailerc.h"
#include "videothumbnailer.h"
#include "filmstripfilter.h"

#include <vector>
#include <stdexcept>

using namespace ffmpegthumbnailer;

struct thumbnailer_data
{
    VideoThumbnailer thumbnailer;
    FilmStripFilter filter;
    thumbnailer_log_callback log_cb = nullptr;
};

static void trace_message(video_thumbnailer* thumbnailer, ThumbnailerLogLevel lvl, const char* message)
{
    if (thumbnailer->tdata->log_cb)
    {
        thumbnailer->tdata->log_cb(lvl, message);
    }
}

extern "C" video_thumbnailer* video_thumbnailer_create(void)
{
    auto thumbnailer = new video_thumbnailer_struct();

    thumbnailer->thumbnail_size             = 128;
    thumbnailer->seek_percentage            = 10;
    thumbnailer->seek_time                  = nullptr;
    thumbnailer->overlay_film_strip         = 0;
    thumbnailer->workaround_bugs            = 0;
    thumbnailer->thumbnail_image_quality    = 8;
    thumbnailer->thumbnail_image_type       = Png;
    thumbnailer->maintain_aspect_ratio      = 1;
    thumbnailer->prefer_embedded_metadata   = 0;
    thumbnailer->av_format_context          = nullptr;
    thumbnailer->tdata                      = new thumbnailer_data();

    return thumbnailer;
}

extern "C" void video_thumbnailer_destroy(video_thumbnailer* thumbnailer)
{
    delete thumbnailer->tdata;
    delete thumbnailer;
}

extern "C" image_data* video_thumbnailer_create_image_data(void)
{
    auto data               = new image_data();

    data->image_data_ptr    = nullptr;
    data->image_data_size   = 0;
    data->image_data_width  = 0;
    data->image_data_height = 0;
    data->internal_data     = new std::vector<uint8_t>();

    return data;
}

extern "C" void video_thumbnailer_destroy_image_data(image_data* data)
{
    data->image_data_ptr    = nullptr;
    data->image_data_size   = 0;
    data->image_data_width  = 0;
    data->image_data_height = 0;

    auto dataVector = reinterpret_cast<std::vector<uint8_t>*>(data->internal_data);
    delete dataVector;
    data->internal_data     = nullptr;

    delete data;
}

static void setProperties(video_thumbnailer* thumbnailer)
{
    auto& videoThumbnailer = thumbnailer->tdata->thumbnailer;

    if (thumbnailer->thumbnail_size >= 0)
    {
        videoThumbnailer.setThumbnailSize(thumbnailer->thumbnail_size);
    }

    videoThumbnailer.setWorkAroundIssues(thumbnailer->workaround_bugs != 0);
    videoThumbnailer.setImageQuality(thumbnailer->thumbnail_image_quality);
    videoThumbnailer.setMaintainAspectRatio(thumbnailer->maintain_aspect_ratio != 0);
    videoThumbnailer.setPreferEmbeddedMetadata(thumbnailer->prefer_embedded_metadata != 0);

    if (thumbnailer->overlay_film_strip)
    {
        videoThumbnailer.removeFilter(&thumbnailer->tdata->filter);
        videoThumbnailer.addFilter(&thumbnailer->tdata->filter);
    }

    if (thumbnailer->seek_time != nullptr)
    {
        videoThumbnailer.setSeekTime(thumbnailer->seek_time);
    }
    else
    {
        videoThumbnailer.setSeekPercentage(thumbnailer->seek_percentage);
    }
}

extern "C" int video_thumbnailer_generate_thumbnail_to_buffer(video_thumbnailer* thumbnailer, const char* movie_filename, image_data* generated_image_data)
{
    try
    {
        auto dataVector         = reinterpret_cast<std::vector<uint8_t>*>(generated_image_data->internal_data);
        auto& videoThumbnailer  = thumbnailer->tdata->thumbnailer;
        setProperties(thumbnailer);
        auto info = videoThumbnailer.generateThumbnail(movie_filename, thumbnailer->thumbnail_image_type, *dataVector, thumbnailer->av_format_context);
        generated_image_data->image_data_ptr = &dataVector->front();
        generated_image_data->image_data_size = static_cast<int>(dataVector->size());
        generated_image_data->image_data_width = info.width;
        generated_image_data->image_data_height = info.height;
        generated_image_data->image_data_source = info.source;
    }
    catch (const std::exception& e)
    {
        trace_message(thumbnailer, ThumbnailerLogLevelError, e.what());
        return -1;
    }

    return 0;
}

extern "C" int video_thumbnailer_generate_thumbnail_to_file(video_thumbnailer* thumbnailer, const char* movie_filename, const char* output_fileName)
{
    try
    {
        auto& videoThumbnailer = thumbnailer->tdata->thumbnailer;
        setProperties(thumbnailer);
        videoThumbnailer.generateThumbnail(movie_filename, thumbnailer->thumbnail_image_type, output_fileName, thumbnailer->av_format_context);
    }
    catch (const std::exception& e)
    {
        trace_message(thumbnailer, ThumbnailerLogLevelError, e.what());
        return -1;
    }

    return 0;
}

extern "C" void video_thumbnailer_set_log_callback(video_thumbnailer* thumbnailer, thumbnailer_log_callback cb)
{
    thumbnailer->tdata->log_cb = cb;
    auto& videoThumbnailer = thumbnailer->tdata->thumbnailer;

    if (!cb)
    {
        videoThumbnailer.setLogCallback(nullptr);
    }
    else
    {
        videoThumbnailer.setLogCallback([=] (ThumbnailerLogLevel lvl, const std::string& msg) {
            cb(lvl, msg.c_str());
        });
    }
}

extern "C" int video_thumbnailer_set_size(video_thumbnailer* thumbnailer, int width, int height)
{
    auto& videoThumbnailer = thumbnailer->tdata->thumbnailer;

    try
    {
        thumbnailer->thumbnail_size = -1;
        videoThumbnailer.setThumbnailSize(width, height);
        return 0;
    }
    catch (const std::exception& e)
    {
        trace_message(thumbnailer, ThumbnailerLogLevelError, e.what());
        return -1;
    }
}
