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

#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif
    
struct video_thumbnailer
{
    int         thumbnail_size;     /* default = 128 */
    int         seek_percentage;    /* default = 10 */
    int         overlay_film_strip; /* default = 0 */
    int         workaround_bugs;    /* default = 0 */
    
    void*       thumbnailer;        /* for internal use only */
};

struct png_data
{
    uint8_t*    png_data_ptr;       /* points to the png data after call to generate_thumbnail_to_buffer */
    int         png_data_size;      /* contains the size of the png data after call to generate_thumbnail_to_buffer */
    
    void*       internal_data;      /* for internal use only */
};

/* intitialize video_thumbnailer structue */
void thumbnailer_init(video_thumbnailer* thumbnailer);
/* destroy video_thumbnailer structue */
void thumbnailer_destroy(video_thumbnailer* thumbnailer);

/* intitialize png_data structue */
void png_data_init(png_data* data);
/* destroy png_data structue */
void png_data_destroy(png_data* data);

/* generate thumbnail from video file (movie_filename), png data is stored in generated_png_data struct */
void generate_thumbnail_to_buffer(video_thumbnailer* thumbnailer, const char* movie_filename, png_data* generated_png_data);
/* generate thumbnail from video file (movie_filename), png is written to output_fileName on disk*/
void generate_thumbnail_to_file(video_thumbnailer* thumbnailer, const char* movie_filename, const char* output_fileName);

#ifdef __cplusplus
}
#endif

#endif
