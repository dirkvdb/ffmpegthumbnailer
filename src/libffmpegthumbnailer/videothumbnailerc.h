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
#include "imagetypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct AVFormatContext;

typedef struct video_thumbnailer_struct
{
    int                     thumbnail_size;         /* default = 128 */
    int                     seek_percentage;        /* default = 10 */
    int                     overlay_film_strip;     /* default = 0 */
    int                     workaround_bugs;        /* default = 0 */
    int                     thumbnail_image_quality;/* default = 8 (0 is bad, 10 is best)*/
    ImageType               thumbnail_image_type;   /* default = Png */
    struct AVFormatContext* av_format_context;      /* default = NULL */
    int                     maintain_aspect_ratio;  /* default = 1 */

    void*               thumbnailer;    /* for internal use only */
} video_thumbnailer;

typedef struct image_data_struct
{
    uint8_t*    image_data_ptr;       /* points to the image data after call to generate_thumbnail_to_buffer */
    int         image_data_size;      /* contains the size of the image data after call to generate_thumbnail_to_buffer */

    void*       internal_data;        /* for internal use only */
} image_data;

/* create video_thumbnailer structure */
video_thumbnailer* create_thumbnailer(void);
/* destroy video_thumbnailer structure */
void destroy_thumbnailer(video_thumbnailer* thumbnailer);

/* create image_data structure */
image_data* create_image_data(void);
/* destroy image_data structure */
void destroy_image_data(image_data* data);

/* generate thumbnail from video file (movie_filename), image data is stored in generated_image_data struct */
int generate_thumbnail_to_buffer(video_thumbnailer* thumbnailer, const char* movie_filename, image_data* generated_image_data);
/* generate thumbnail from video file (movie_filename), image is written to output_fileName on disk*/
int generate_thumbnail_to_file(video_thumbnailer* thumbnailer, const char* movie_filename, const char* output_fileName);

#ifdef __cplusplus
}
#endif

#endif
