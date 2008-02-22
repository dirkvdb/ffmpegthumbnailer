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

#include <iostream>
#include <unistd.h>
#include <stdexcept>

#include "libffmpegthumbnailer/videothumbnailer.hpp"

using namespace std;

void printUsage();

int main(int argc, char** argv)
{
	int 	option;
	int 	seekPercentage = 10;
	int 	thumbnailSize = 128;
	bool 	filmStripOverlay = false;
    bool    workaroundIssues = false;
	char* 	inputFile = NULL;
	char* 	outputFile = NULL;
	
	while ((option = getopt (argc, argv, "i:o:s:t:fwh")) != -1)
	{
		switch (option)
		{
			case 'i':
				inputFile = optarg;
				break;
			case 'o':
				outputFile = optarg;
				break;
			case 's':
				thumbnailSize = atoi(optarg);
				break;
			case 'f':
				filmStripOverlay = true;
				break;
			case 't':
				seekPercentage = atoi(optarg);
				break;
			case 'w':
				workaroundIssues = true;
				break;
            case 'h':
				printUsage();
				return 0;
			case '?':
			default:
				cerr << "invalid arguments" << endl;
				printUsage();
				return -1;
		}
	}
	
	if (!inputFile || !outputFile)
	{
		cerr << "invalid arguments" << endl;
		printUsage();
        return 0;
	}
    
	try
	{
		VideoThumbnailer videoThumbnailer(thumbnailSize, seekPercentage, filmStripOverlay, workaroundIssues);
		videoThumbnailer.generateThumbnail(inputFile, outputFile);
	}
	catch (exception& e)
	{
		cerr << "Error: " << e.what() << endl;
		return -1;
	}
	catch (...)
	{
		return -1;
	}
	
	return 0;
}

void printUsage()
{
	cout << "Usage: ffmpegvideothumbnailer [options]" << endl << endl
		 << "Options:" << endl
		 << "  -i<s>  : input file" << endl
		 << "  -o<s>  : output file" << endl
		 << "  -s<n>  : thumbnail size (default: 128)" << endl
		 << "  -t<n>  : time to seek to (percentage) (default: 10)" << endl
		 << "  -f     : create a movie strip overlay" << endl
         << "  -w     : workaround issues in old versions of ffmpeg" << endl
         << "  -h     : display this help" << endl;
}
