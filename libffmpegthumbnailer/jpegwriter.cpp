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

#include "jpegwriter.h"
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>

using namespace std;

namespace ffmpegthumbnailer
{

struct BufferWriter
{
    jpeg_destination_mgr    m_DestMgr;
    uint8_t*                m_pDataBuffer;
    std::vector<uint8_t>*   m_pDataSink;
};

const static int JPEG_WORK_BUFFER_SIZE = 8192;

static void jpegInitDestination(j_compress_ptr pCompressionInfo);
static boolean jpegFlushWorkBuffer(j_compress_ptr pCompressionInfo);
static void jpegDestroyDestination(j_compress_ptr pCompressionInfo);

JpegWriter::JpegWriter(const string& outputFile)
: ImageWriter()
, m_pFile(nullptr)
, m_pBufferWriter(nullptr)
{
    init();
	m_pFile = outputFile == "-" ? stdout : fopen(outputFile.c_str(), "wb");

    if (!m_pFile)
    {
       throw logic_error(string("Failed to open output file: ") + outputFile);
    }

    jpeg_stdio_dest(&m_Compression, m_pFile);
}

JpegWriter::JpegWriter(std::vector<uint8_t>& outputBuffer)
: ImageWriter()
, m_pFile(nullptr)
, m_pBufferWriter(nullptr)
{
    init();

    m_Compression.dest = (jpeg_destination_mgr*)(m_Compression.mem->alloc_small) ((j_common_ptr) &m_Compression, JPOOL_PERMANENT, sizeof(BufferWriter));

    m_pBufferWriter = reinterpret_cast<BufferWriter*>(m_Compression.dest);
    m_pBufferWriter->m_DestMgr.init_destination     = jpegInitDestination;
    m_pBufferWriter->m_DestMgr.empty_output_buffer  = jpegFlushWorkBuffer;
    m_pBufferWriter->m_DestMgr.term_destination     = jpegDestroyDestination;
    m_pBufferWriter->m_pDataSink                    = &outputBuffer;
}

JpegWriter::~JpegWriter()
{
    if (m_pFile)
    {
        fclose(m_pFile);
    }

    jpeg_destroy_compress(&m_Compression);
}

void JpegWriter::init()
{
    m_Compression.err = jpeg_std_error(&m_ErrorHandler);
    jpeg_create_compress(&m_Compression);
}

void JpegWriter::setText(const string& /*key*/, const string& /*value*/)
{
}

void JpegWriter::writeFrame(uint8_t** rgbData, int width, int height, int quality)
{
    m_Compression.image_width = width;
    m_Compression.image_height = height;
    m_Compression.input_components = 3;
    m_Compression.in_color_space = JCS_RGB;

    quality = min(10, quality);
    quality = max(0, quality);

    jpeg_set_defaults(&m_Compression);
    jpeg_set_quality(&m_Compression, quality * 10, TRUE);
    jpeg_start_compress(&m_Compression, TRUE);

    while (m_Compression.next_scanline < m_Compression.image_height)
    {
        (void) jpeg_write_scanlines(&m_Compression, reinterpret_cast<JSAMPLE**>(&rgbData[m_Compression.next_scanline]), 1);
    }

    jpeg_finish_compress(&m_Compression);
}

void jpegInitDestination(j_compress_ptr pCompressionInfo)
{
    auto bufWriter = reinterpret_cast<BufferWriter*>(pCompressionInfo->dest);

    bufWriter->m_pDataBuffer = (uint8_t*)(*pCompressionInfo->mem->alloc_small) ((j_common_ptr) pCompressionInfo, JPOOL_IMAGE, JPEG_WORK_BUFFER_SIZE);
    bufWriter->m_DestMgr.next_output_byte = bufWriter->m_pDataBuffer;
    bufWriter->m_DestMgr.free_in_buffer = JPEG_WORK_BUFFER_SIZE;
}

boolean jpegFlushWorkBuffer(j_compress_ptr pCompressionInfo)
{
    auto bufWriter = reinterpret_cast<BufferWriter*>(pCompressionInfo->dest);

    size_t prevSize = bufWriter->m_pDataSink->size();
    bufWriter->m_pDataSink->resize(prevSize + JPEG_WORK_BUFFER_SIZE);
    memcpy((&bufWriter->m_pDataSink->front()) + prevSize, bufWriter->m_pDataBuffer, JPEG_WORK_BUFFER_SIZE);

    bufWriter->m_DestMgr.next_output_byte = bufWriter->m_pDataBuffer;
    bufWriter->m_DestMgr.free_in_buffer = JPEG_WORK_BUFFER_SIZE;

    return TRUE;
}


void jpegDestroyDestination(j_compress_ptr pCompressionInfo)
{
    auto bufWriter = reinterpret_cast<BufferWriter*>(pCompressionInfo->dest);
    size_t datacount = JPEG_WORK_BUFFER_SIZE - bufWriter->m_DestMgr.free_in_buffer;

    size_t prevSize = bufWriter->m_pDataSink->size();
    bufWriter->m_pDataSink->resize(prevSize + datacount);
    memcpy((&bufWriter->m_pDataSink->front()) + prevSize, bufWriter->m_pDataBuffer, datacount);
}

}
