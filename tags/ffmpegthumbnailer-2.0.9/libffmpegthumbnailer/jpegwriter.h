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

#ifndef JPEG_WRITER_H
#define JPEG_WRITER_H

#include <string>
#include <vector>

extern "C" {
    #include <jpeglib.h>
}

#include "imagewriter.h"

namespace ffmpegthumbnailer
{

struct BufferWriter;

class JpegWriter : public ImageWriter
{
public:
    JpegWriter(const std::string& outputFile);
    JpegWriter(std::vector<uint8_t>& outputBuffer);
    ~JpegWriter();
    
    void setText(const std::string& key, const std::string& value);
    void writeFrame(uint8_t** rgbData, int width, int height, int quality);
    
private:
    void init();
    
private:
    FILE*                   m_pFile;
    jpeg_compress_struct    m_Compression;
    jpeg_error_mgr          m_ErrorHandler;
    BufferWriter*           m_pBufferWriter;
};

}

#endif
