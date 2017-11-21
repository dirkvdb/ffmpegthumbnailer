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

#ifndef VIDEO_THUMBNAILERC_H
#define VIDEO_THUMBNAILERC_H

#include <inttypes.h>
#include "ffmpegthumbnailertypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct AVFormatContext;
struct thumbnailer_data;

typedef void(*thumbnailer_log_callback)(ThumbnailerLogLevel, const char*);

typedef struct video_thumbnailer_struct
{
    int                         thumbnail_size __attribute__((deprecated("use video_thumbnailer_set_size()"))); /* default = 128 (deprecated, use video_thumbnailer_set_size)*/
    int                         seek_percentage;            /* default = 10 */
    char*                       seek_time;                  /* default = NULL (format hh:mm:ss, overrides seek_percentage if set) */
    int                         overlay_film_strip;         /* default = 0 */
    int                         workaround_bugs;            /* default = 0 */
    int                         thumbnail_image_quality;    /* default = 8 (0 is bad, 10 is best)*/
    ThumbnailerImageType        thumbnail_image_type;       /* default = Png */
    struct AVFormatContext*     av_format_context;          /* default = NULL */
    int                         maintain_aspect_ratio;      /* default = 1 */
    int                         prefer_embedded_metadata;   /* default = 0 */

    struct thumbnailer_data*    tdata;                      /* for internal use only */
} video_thumbnailer;

typedef struct image_data_struct
{
    uint8_t*                image_data_ptr;       /* points to the image data after call to generate_thumbnail_to_buffer */
    int                     image_data_size;      /* contains the size of the image data after call to generate_thumbnail_to_buffer (=width*height*3)*/
    int                     image_data_width;     /* contains the width of the image data after call to generate_thumbnail_to_buffer */
    int                     image_data_height;    /* contains the height of the image data after call to generate_thumbnail_to_buffer */
    ThumbnailerImageSource  image_data_source;    /* contains the source that was used to generate the thumbnail */

    void*       internal_data;        /* for internal use only */
} image_data;

/* create video_thumbnailer structure */
video_thumbnailer* video_thumbnailer_create(void);
/* destroy video_thumbnailer structure */
void video_thumbnailer_destroy(video_thumbnailer* thumbnailer);

/* create image_data structure */
image_data* video_thumbnailer_create_image_data(void);
/* destroy image_data structure */
void video_thumbnailer_destroy_image_data(image_data* data);

/* generate thumbnail from video file (movie_filename), image data is stored in generated_image_data struct */
int video_thumbnailer_generate_thumbnail_to_buffer(video_thumbnailer* thumbnailer, const char* movie_filename, image_data* generated_image_data);
/* generate thumbnail from video file (movie_filename), image is written to output_fileName on disk*/
int video_thumbnailer_generate_thumbnail_to_file(video_thumbnailer* thumbnailer, const char* movie_filename, const char* output_fileName);
/* install a logging callback that gets called on errors and informational messages, reset by passing NULL.
   by default no logging is generated on stdout or stderr */
void video_thumbnailer_set_log_callback(video_thumbnailer* thumbnailer, thumbnailer_log_callback cb);
/* Finer control of the generate image thumbnail size
   Using this function overrides the size setting of the video_thumbnailer struct
   Use a value of 0 or less to ignore the setting:
   e.g. Calling this function with 0 and 100 as arguments will set the width using the video aspect ratio */
int video_thumbnailer_set_size(video_thumbnailer* thumbnailer, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
