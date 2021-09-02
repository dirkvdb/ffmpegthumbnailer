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

#include "rgbwriter.h"
#include <stdexcept>
#include <cassert>
#include <cstring>

using namespace std;

namespace ffmpegthumbnailer
{

RgbWriter::RgbWriter(const string& outputFile)
: ImageWriter()
, m_OutputBuffer(nullptr)
{
    m_FilePtr = outputFile == "-" ? stdout : fopen(outputFile.c_str(), "wb");

    if (!m_FilePtr)
    {
       throw logic_error(string("Failed to open output file: ") + outputFile);
    }
}

RgbWriter::RgbWriter(std::vector<uint8_t>& outputBuffer)
: ImageWriter()
, m_FilePtr(nullptr)
, m_OutputBuffer(&outputBuffer)
{
}

RgbWriter::~RgbWriter()
{
    if (m_FilePtr)
    {
        fclose(m_FilePtr);
    }
}

void RgbWriter::setText(const string& /*key*/, const string& /*value*/)
{
}

void RgbWriter::writeFrame(uint8_t** rgbData, int width, int height, int /*quality*/)
{
    const auto lineSize = static_cast<size_t>(width * 3);

    if (m_FilePtr)
    {
        for (int i = 0; i < height; ++i)
        {
            fwrite(rgbData[i], sizeof(uint8_t), lineSize, m_FilePtr);
        }
    }
    else 
    {
        m_OutputBuffer->resize(width * height * 3);

        for (int i = 0; i < height; ++i)
        {
            memcpy(m_OutputBuffer->data() + (lineSize * i), rgbData[i], lineSize);
        }
    }
}

}
