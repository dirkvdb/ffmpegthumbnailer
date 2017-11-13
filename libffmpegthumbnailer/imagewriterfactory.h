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

#ifndef IMAGE_WRITER_FACTORY_H
#define IMAGE_WRITER_FACTORY_H

#include "config.h"

#include "imagewriter.h"
#include "imagetypes.h"
#include "rgbwriter.h"

#ifdef HAVE_PNG
#include "pngwriter.h"
#endif

#ifdef HAVE_JPEG
#include "jpegwriter.h"
#endif


#include <string>
#include <inttypes.h>
#include <stdexcept>

namespace ffmpegthumbnailer
{

template <typename T>
class ImageWriterFactory
{
public:
    static ImageWriter* createImageWriter(ThumbnailerImageType imageType, T output)
    {
        switch (imageType)
        {
            case Png:
#ifdef HAVE_PNG
                return new PngWriter(output);
#else
                throw std::logic_error("ffmpegthumbnailer was not compiled with png support");
#endif
            case Jpeg:
#ifdef HAVE_JPEG
                return new JpegWriter(output);
#else
                throw std::logic_error("ffmpegthumbnailer was not compiled with jpeg support");
#endif
            case Rgb:
                return new RgbWriter(output);
            default:
                throw std::logic_error("ImageWriterFactory::createImageWriter: Invalid image type specified");
        }
    }
};

}

#endif
