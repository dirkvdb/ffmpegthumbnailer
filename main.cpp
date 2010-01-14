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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <stdlib.h>

#include "libffmpegthumbnailer/videothumbnailer.h"
#include "libffmpegthumbnailer/stringoperations.h"
#include "libffmpegthumbnailer/filmstripfilter.h"

using namespace std;
using namespace ffmpegthumbnailer;

void printVersion();
void printUsage();
ThumbnailerImageType determineImageTypeFromString(const std::string& filename);
ThumbnailerImageType determineImageTypeFromFilename(const std::string& filename);

int main(int argc, char** argv)
{
    int     option;
    int     seekPercentage = 10;
    string  seekTime;
    int     thumbnailSize = 128;
    int     imageQuality = 8;
    bool    filmStripOverlay = false;
    bool    workaroundIssues = false;
    bool    maintainAspectRatio = true;
    bool    smartFrameSelection = false;
    string  inputFile;
    string  outputFile;
    string  imageFormat;

    while ((option = getopt (argc, argv, "i:o:s:t:q:c:afwhvp")) != -1)
    {
        switch (option)
        {
            case 'a':
                maintainAspectRatio = false;
                break;
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
            case 'p':
                smartFrameSelection = true;
                break;
            case 't':
                if (string(optarg).find(':') != string::npos)
                {
                    seekTime = optarg;
                }
                else
                {
                    seekPercentage = atoi(optarg);
                }
                break;
            case 'w':
                workaroundIssues = true;
                break;
            case 'q':
                imageQuality = atoi(optarg);
                break;
            case 'c':
                imageFormat = optarg;
                break;
            case 'h':
                printUsage();
                return 0;
            case 'v':
                printVersion();
                return 0;
            case '?':
            default:
                cerr << "invalid arguments" << endl;
                printUsage();
                return -1;
        }
    }

    if (inputFile.empty() || outputFile.empty())
    {
        cerr << "invalid arguments" << endl;
        printUsage();
        return 0;
    }
    
    try
    {
        ThumbnailerImageType imageType = imageFormat.empty() ?
              determineImageTypeFromFilename(outputFile)
            : determineImageTypeFromString(imageFormat);
    
        VideoThumbnailer videoThumbnailer(thumbnailSize, workaroundIssues, maintainAspectRatio, imageQuality, smartFrameSelection);
        FilmStripFilter* filmStripFilter = NULL;

        if (filmStripOverlay)
        {
            filmStripFilter = new FilmStripFilter();
            videoThumbnailer.addFilter(filmStripFilter);
        }

        if (!seekTime.empty())
        {
            videoThumbnailer.setSeekTime(seekTime);
        }
        else
        {
            videoThumbnailer.setSeekPercentage(seekPercentage);
        }
        videoThumbnailer.generateThumbnail(inputFile, imageType, outputFile);

        delete filmStripFilter;
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

void printVersion()
{
    cout << PACKAGE_NAME" version: "VERSION << endl;
}

void printUsage()
{
    cout << "Usage: "PACKAGE_NAME" [options]" << endl << endl
         << "Options:" << endl
         << "  -i<s>   : input file" << endl
         << "  -o<s>   : output file" << endl
         << "  -s<n>   : thumbnail size (use 0 for original size) (default: 128)" << endl
         << "  -t<n|s> : time to seek to (percentage or absolute time hh:mm:ss) (default: 10%)" << endl
         << "  -q<n>   : image quality (0 = bad, 10 = best) (default: 8)" << endl
         << "  -c      : override image format (jpeg or png) (default: determined by filename)" << endl
         << "  -a      : ignore aspect ratio and generate square thumbnail" << endl
         << "  -f      : create a movie strip overlay" << endl
         //<< "  -p      : use smarter frame selection (slower)" << endl
         << "  -w      : workaround issues in old versions of ffmpeg" << endl
         << "  -v      : print version number" << endl
         << "  -h      : display this help" << endl;
}

ThumbnailerImageType determineImageTypeFromString(const std::string& type)
{
    string lowercaseType = type;
    StringOperations::lowercase(lowercaseType);
    
    if (lowercaseType == "png")
    {
        return Png;
    }
    
    if (lowercaseType == "jpeg" || lowercaseType == "jpg")
    {
        return Jpeg;
    }
    
    throw logic_error("Invalid image type specified");
}

ThumbnailerImageType determineImageTypeFromFilename(const std::string& filename)
{
    string lowercaseFilename = filename;
    StringOperations::lowercase(lowercaseFilename);

    size_t size = lowercaseFilename.size();
    if ((lowercaseFilename.substr(size - 5, size) == ".jpeg") || (lowercaseFilename.substr(size - 4, size) == ".jpg"))
    {
        return Jpeg;
    }

    return Png;
}
