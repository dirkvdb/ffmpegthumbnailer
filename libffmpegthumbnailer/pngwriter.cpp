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

#include "pngwriter.h"
#include <stdexcept>
#include <cassert>
#include <cstring>

using namespace std;

namespace ffmpegthumbnailer
{

static void writeDataCallback(png_structp png_ptr, png_bytep data, png_size_t length);

PngWriter::PngWriter(const string& outputFile)
: ImageWriter()
, m_FilePtr(nullptr)
, m_PngPtr(nullptr)
, m_InfoPtr(nullptr)
{
    init();
    m_FilePtr = outputFile == "-" ? stdout : fopen(outputFile.c_str(), "wb");

    if (!m_FilePtr)
    {
       throw logic_error(string("Failed to open output file: ") + outputFile);
    }

    png_init_io(m_PngPtr, m_FilePtr);
}

PngWriter::PngWriter(std::vector<uint8_t>& outputBuffer)
: ImageWriter()
, m_FilePtr(nullptr)
, m_PngPtr(nullptr)
, m_InfoPtr(nullptr)
{
    init();
    png_set_write_fn(m_PngPtr, (png_voidp)&outputBuffer, writeDataCallback, nullptr);
}

PngWriter::~PngWriter()
{
    if (m_FilePtr)
    {
        fclose(m_FilePtr);
    }
    png_destroy_write_struct(&m_PngPtr, &m_InfoPtr);
}

void PngWriter::init()
{
    m_PngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!m_PngPtr)
    {
        throw logic_error("Failed to create png write structure");
    }

    m_InfoPtr = png_create_info_struct(m_PngPtr);
    if (!m_InfoPtr)
    {
        png_destroy_write_struct(&m_PngPtr, (png_infopp) nullptr);
        throw logic_error("Failed to create png info structure");
    }
}

void PngWriter::setText(const string& key, const string& value)
{
    png_text pngText;

    pngText.compression = -1;
    pngText.key = const_cast<char*>(key.c_str());
    pngText.text = const_cast<char*>(value.c_str());

    png_set_text(m_PngPtr, m_InfoPtr, &pngText, 1);
}

void PngWriter::writeFrame(uint8_t** rgbData, int width, int height, int /*quality*/)
{
    if (setjmp(png_jmpbuf(m_PngPtr)))
    {
        throw logic_error("Writing png file failed");
    }

    png_set_IHDR(m_PngPtr, m_InfoPtr, width, height, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_rows(m_PngPtr, m_InfoPtr, rgbData);
    png_write_png(m_PngPtr, m_InfoPtr, 0, nullptr);
}

void writeDataCallback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    vector<uint8_t>& outputBuffer = *(reinterpret_cast<vector<uint8_t>* >(png_get_io_ptr(png_ptr)));
    int prevBufSize = outputBuffer.size();
    outputBuffer.resize(outputBuffer.size() + length);
    memcpy(&outputBuffer[prevBufSize], data, length);
}

}
