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

#ifndef PNG_WRITER_H
#define PNG_WRITER_H

#include <string>
#include <vector>
#include <png.h>

#include "imagewriter.hpp"

class PngWriter : public ImageWriter
{
public:
	PngWriter(const std::string& outputFile);
    PngWriter(std::vector<uint8_t>& outputBuffer);
	~PngWriter();
	
	void setText(const std::string& key, const std::string& value);
	void writeFrame(png_byte** rgbData, int width, int height);
	
private:
    void initPng();
    static void writeRowCallback(png_structp, png_uint_32, int);
    static void writeDataCallback(png_structp png_ptr, png_bytep data, png_size_t length);
	
private:
	FILE* 		            m_FilePtr;
	png_structp             m_PngPtr;
	png_infop 	            m_InfoPtr;
};

#endif
