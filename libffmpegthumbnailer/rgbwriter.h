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

#ifndef RGB_WRITER_H
#define RGB_WRITER_H

#include <string>
#include <vector>
#include <fstream>

#include "imagewriter.h"

namespace ffmpegthumbnailer
{

class RgbWriter : public ImageWriter
{
public:
    RgbWriter(const std::string& outputFile);
    RgbWriter(std::vector<uint8_t>& outputBuffer);
    ~RgbWriter();

    void setText(const std::string& key, const std::string& value) override;
    void writeFrame(uint8_t** rgbData, int width, int height, int quality) override;

private:
    FILE*                   m_FilePtr;
    std::vector<uint8_t>*   m_OutputBuffer;
};

}

#endif
