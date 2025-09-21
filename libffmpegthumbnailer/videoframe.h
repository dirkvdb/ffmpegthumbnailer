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

#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H

#include "ffmpegthumbnailertypes.h"

#include <cinttypes>
#include <vector>

namespace ffmpegthumbnailer {

struct VideoFrame
{
    VideoFrame()
    : width(0), height(0), lineSize(0)
    {
    }

    VideoFrame(int width, int height, int lineSize, ThumbnailerImageSource source)
    : width(width), height(height), lineSize(lineSize), imageSource(source)
    {
    }

    int width;
    int height;
    int lineSize;

    std::vector<uint8_t> frameData;
    ThumbnailerImageSource imageSource;
};

}

#endif
