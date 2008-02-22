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

#include <stdlib.h>

#include "libffmpegthumbnailer/videothumbnailerc.h"

int main(int argc, char** argv)
{
    struct video_thumbnailer thumbnailer;
    struct image_data imageData;
    
    thumbnailer_init(&thumbnailer)
    image_data_init(&imageData)
    
    thumbnailer.seek_percentage    = 15;
    thumbnailer.overlay_film_strip = 1;
    
    generate_thumbnail_to_buffer(thumbnailer, "my.movie.avi", &imageData);
    
    FILE* pngFile = fopen("output.png", "wb");
    fwrite(imageData.image_data_ptr, 1 , imageData.image_data_size, pngFile);    
    fclose(pngFile);
            
    image_data_destroy(&imageData)
    thumbnailer_destroy(&thumbnailer)

	return 0;
}
