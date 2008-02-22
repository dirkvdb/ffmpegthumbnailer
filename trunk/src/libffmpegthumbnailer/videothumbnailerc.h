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

#ifndef VIDEO_THUMBNAILERC_H
#define VIDEO_THUMBNAILERC_H

#include <stdlib.h>
#include <inttypes.h>

extern "C"
{
    struct video_thumbnailer
    {
        int         thumbnail_size;
        int         seek_percentage;
        int         overlay_film_strip;
        int         workaround_bugs;
        
        void*       thumbnailer;
    };
    
    struct png_data
    {
        uint8_t*    png_data_ptr;
        int         png_data_size;
        
        void*       internal_data;
    };
    
    void thumbnailer_init(video_thumbnailer* thumbnailer);
    void thumbnailer_destroy(video_thumbnailer* thumbnailer);
    
    void png_data_init(png_data* data);
    void png_data_destroy(png_data* data);
    
    void generate_thumbnail_to_buffer(video_thumbnailer* thumbnailer, const char* movie_filename, png_data* generated_png_data);
    void generate_thumbnail_to_file(video_thumbnailer* thumbnailer, const char* movie_filename, const char* output_fileName);
}

#endif
