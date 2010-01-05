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

#ifndef GRAYSCALEFILTER_H
#define GRAYSCALEFILTER_H

#include "videoframe.h"

namespace ffmpegthumbnailer
{

class GrayScaleFilter : public IFilter
{
public:
    void process(VideoFrame& videoFrame)
    {
        for (int i = 0; i < videoFrame.height; ++i)
        {
            for (int j = 0; j < videoFrame.width; ++j)
            {
                int pixelIndex = (i * videoFrame.lineSize) + (j * 3);
                uint8_t grayValue = (  videoFrame.frameData[pixelIndex]
                                     + videoFrame.frameData[pixelIndex+1]
                                     + videoFrame.frameData[pixelIndex+2] ) / 3;

                videoFrame.frameData[pixelIndex] = grayValue;
                videoFrame.frameData[pixelIndex+1] = grayValue;
                videoFrame.frameData[pixelIndex+2] = grayValue;
            }
        }
    }
};

}

#endif
