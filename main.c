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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "libffmpegthumbnailer/videothumbnailerc.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("No moviefile specified\n");
        return -1;
    }

    video_thumbnailer* thumbnailer = video_thumbnailer_create();
    image_data* imageData = video_thumbnailer_create_image_data();

    thumbnailer->seek_percentage        = 15;
    thumbnailer->overlay_film_strip     = 1;
    thumbnailer->thumbnail_size         = 256;
    thumbnailer->thumbnail_image_type   = Jpeg;

    if (video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, argv[1], imageData) == 0)
    {
        FILE* imageFile = fopen("output.jpg", "wb");
        fwrite(imageData->image_data_ptr, 1 , imageData->image_data_size, imageFile);
        fclose(imageFile);
    }
    else
    {
        printf("Failed to generate thumbnail\n");
    }

    video_thumbnailer_destroy_image_data(imageData);
    video_thumbnailer_destroy(thumbnailer);

    return 0;
}
