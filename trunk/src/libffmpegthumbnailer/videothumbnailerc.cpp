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

#include "videothumbnailerc.h"
#include "videothumbnailer.hpp"

#include <vector>

extern "C" void thumbnailer_init(video_thumbnailer* thumbnailer)
{
    thumbnailer->thumbnailer        = new VideoThumbnailer();
    thumbnailer->thumbnail_size     = 128;
    thumbnailer->seek_percentage    = 10;
    thumbnailer->overlay_film_strip = 0;
    thumbnailer->workaround_bugs    = 0;
}

extern "C" void thumbnailer_destroy(video_thumbnailer* thumbnailer)
{
    VideoThumbnailer* videoThumbnailer = reinterpret_cast<VideoThumbnailer*>(thumbnailer->thumbnailer);
    delete videoThumbnailer;
    thumbnailer->thumbnailer = 0;
}

extern "C" void png_data_init(png_data* data)
{
    data->png_data_ptr = 0;
    data->png_data_size = 0;
    data->internal_data = new std::vector<uint8_t>();
}

extern "C" void png_data_destroy(png_data* data)
{
    data->png_data_ptr = 0;
    data->png_data_size = 0;
    
    std::vector<uint8_t>* dataVector = reinterpret_cast<std::vector<uint8_t>* >(data->internal_data);
    delete dataVector;
    data->internal_data = 0;
}

extern "C" void generate_thumbnail_to_buffer(video_thumbnailer* thumbnailer, const char* movie_filename, png_data* generated_png_data)
{
    VideoThumbnailer* videoThumbnailer  = reinterpret_cast<VideoThumbnailer*>(thumbnailer->thumbnailer);
    std::vector<uint8_t>* dataVector    = reinterpret_cast<std::vector<uint8_t>* >(generated_png_data->internal_data);
    
    videoThumbnailer->setThumbnailSize(thumbnailer->thumbnail_size);
    videoThumbnailer->setSeekPercentage(thumbnailer->seek_percentage);
    videoThumbnailer->setFilmStripOverlay(thumbnailer->overlay_film_strip != 0);
    videoThumbnailer->setWorkAroundIssues(thumbnailer->workaround_bugs != 0);
    
    videoThumbnailer->generateThumbnail(movie_filename, *dataVector);
    generated_png_data->png_data_ptr = &dataVector->front();
    generated_png_data->png_data_size = dataVector->size();
}

extern "C" void generate_thumbnail_to_file(video_thumbnailer* thumbnailer, const char* movie_filename, const char* output_fileName)
{
    VideoThumbnailer* videoThumbnailer = reinterpret_cast<VideoThumbnailer*>(thumbnailer->thumbnailer);
    
    videoThumbnailer->setThumbnailSize(thumbnailer->thumbnail_size);
    videoThumbnailer->setSeekPercentage(thumbnailer->seek_percentage);
    videoThumbnailer->setFilmStripOverlay(thumbnailer->overlay_film_strip != 0);
    videoThumbnailer->setWorkAroundIssues(thumbnailer->workaround_bugs != 0);
    
    videoThumbnailer->generateThumbnail(movie_filename, output_fileName);
}
