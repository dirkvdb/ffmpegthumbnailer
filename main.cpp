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

#include "config.h"

#include <clocale>
#include <iostream>
#include <stdexcept>
#include <stdlib.h>
#include <thread>
#include <unistd.h>

#ifdef ENABLE_GIO
#include <dlfcn.h>
#endif

#include "libffmpegthumbnailer/filmstripfilter.h"
#include "libffmpegthumbnailer/stringoperations.h"
#include "libffmpegthumbnailer/videothumbnailer.h"

using namespace ffmpegthumbnailer;

void                 printVersion();
void                 printUsage();
void                 tryUriConvert(std::string& filename);
ThumbnailerImageType determineImageTypeFromString(const std::string& filename);
ThumbnailerImageType determineImageTypeFromFilename(const std::string& filename);

int main(int argc, char** argv)
{
    int         option;
    int         seekPercentage = 10;
    std::string seekTime;
    std::string thumbnailSize          = "128";
    int         imageQuality           = 8;
    bool        filmStripOverlay       = false;
    bool        workaroundIssues       = false;
    bool        maintainAspectRatio    = true;
    bool        smartFrameSelection    = false;
    bool        preferEmbeddedMetadata = false;
    std::string inputFile;
    std::string outputFile;
    std::string imageFormat;

    if (!std::setlocale(LC_CTYPE, ""))
    {
        std::cerr << "Failed to set locale" << std::endl;
    }

    while ((option = getopt(argc, argv, "i:o:s:t:q:c:r:afwhvpm")) != -1)
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
            thumbnailSize = optarg;
            break;
        case 'f':
            filmStripOverlay = true;
            break;
        case 'p':
            smartFrameSelection = true;
            break;
        case 'm':
            preferEmbeddedMetadata = true;
            break;
        case 't':
            if (std::string(optarg).find(':') != std::string::npos)
            {
                seekTime = optarg;
            }
            else
            {
                seekPercentage = std::atoi(optarg);
            }
            break;
        case 'w':
            workaroundIssues = true;
            break;
        case 'q':
            imageQuality = std::atoi(optarg);
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
            std::cerr << "invalid arguments" << std::endl;
            printUsage();
            return -1;
        }
    }

    if (inputFile.empty() || outputFile.empty())
    {
        std::cerr << "invalid arguments" << std::endl;
        printUsage();
        return 0;
    }

    if (outputFile == "-" && imageFormat.empty())
    {
        std::cerr << "When writing to stdout the image format needs to be specified (e.g.: -c png)" << std::endl;
        return 0;
    }

    try
    {
#ifdef ENABLE_GIO
        tryUriConvert(inputFile);
#endif
        ThumbnailerImageType imageType = imageFormat.empty() ? determineImageTypeFromFilename(outputFile)
                                                             : determineImageTypeFromString(imageFormat);

        VideoThumbnailer videoThumbnailer(0, workaroundIssues, maintainAspectRatio, imageQuality, smartFrameSelection);
        videoThumbnailer.setThumbnailSize(thumbnailSize);
        videoThumbnailer.setLogCallback([](ThumbnailerLogLevel lvl, const std::string& msg) {
            std::cerr << msg << std::endl;
        });

        std::unique_ptr<FilmStripFilter> filmStripFilter;

        if (filmStripOverlay)
        {
            filmStripFilter.reset(new FilmStripFilter());
            videoThumbnailer.addFilter(filmStripFilter.get());
        }

        videoThumbnailer.setPreferEmbeddedMetadata(preferEmbeddedMetadata);

        if (!seekTime.empty())
        {
            videoThumbnailer.setSeekTime(seekTime);
        }
        else
        {
            videoThumbnailer.setSeekPercentage(seekPercentage);
        }

        videoThumbnailer.generateThumbnail(inputFile, imageType, outputFile);
    }
    catch (std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unexpected rror" << std::endl;
        return -1;
    }

    return 0;
}

void printVersion()
{
    std::cout << PACKAGE " version: " PACKAGE_VERSION "\n";
}

void printUsage()
{
    std::cout << "Usage: " PACKAGE " [options]\n\n"
              << "Options:\n"
              << "  -i<s>   : input file\n"
              << "  -o<s>   : output file\n"
              << "  -s<n>   : thumbnail size (use 0 for original size) (default: 128)\n"
              << "  -t<n|s> : time to seek to (percentage or absolute time hh:mm:ss) (default: 10%)\n"
              << "  -q<n>   : image quality (0 = bad, 10 = best) (default: 8)\n"
              << "  -c      : override image format (jpeg, png or rgb) (default: determined by filename)\n"
              << "  -a      : ignore aspect ratio and generate square thumbnail\n"
              << "  -f      : create a movie strip overlay\n"
              //<< "  -p      : use smarter frame selection (slower)\n"
              << "  -m      : prefer embedded image metadata over video content\n"
              << "  -w      : workaround issues in old versions of ffmpeg\n"
              << "  -v      : print version number\n"
              << "  -h      : display this help\n";
}

ThumbnailerImageType determineImageTypeFromString(const std::string& type)
{
    std::string lowercaseType = type;
    StringOperations::lowercase(lowercaseType);

    if (lowercaseType == "png")
    {
        return Png;
    }

    if (lowercaseType == "jpeg" || lowercaseType == "jpg")
    {
        return Jpeg;
    }

    if (lowercaseType == "raw" || lowercaseType == "rgb")
    {
        return Rgb;
    }

    throw std::logic_error("Invalid image type specified");
}

#ifdef ENABLE_GIO
typedef void* (*FileCreateFunc)(const char*);
typedef char* (*FileGetFunc)(void* file);
typedef int (*IsNativeFunc)(void* file);
typedef void (*InitFunc)(void);
typedef void (*FreeFunc)(void*);
typedef void (*UnrefFunc)(void*);

class LibHandle
{
public:
    LibHandle(const std::string& libName)
    : m_pLib(dlopen(libName.c_str(), RTLD_LAZY))
    {
        if (!m_pLib) std::cerr << dlerror() << std::endl;
    }

    ~LibHandle()
    {
        if (m_pLib) dlclose(m_pLib);
    }

    operator void*() const { return m_pLib; }
    operator bool() const { return m_pLib != nullptr; }

private:
    void* m_pLib;
};

void tryUriConvert(std::string& filename)
{
    if (filename.find(":") == std::string::npos)
    {
        return;
    }

    LibHandle gLib("libglib-2.0.so.0");
    LibHandle gobjectLib("libgobject-2.0.so.0");
    LibHandle gioLib("libgio-2.0.so.0");

    if (gioLib && gLib && gobjectLib)
    {
        FileCreateFunc createUriFunc = (FileCreateFunc)dlsym(gioLib, "g_file_new_for_uri");

        IsNativeFunc nativeFunc = (IsNativeFunc)dlsym(gioLib, "g_file_is_native");
        FileGetFunc  getFunc    = (FileGetFunc)dlsym(gioLib, "g_file_get_path");
        InitFunc     initFunc   = (InitFunc)dlsym(gobjectLib, "g_type_init");
        UnrefFunc    unrefFunc  = (UnrefFunc)dlsym(gobjectLib, "g_object_unref");
        FreeFunc     freeFunc   = (FreeFunc)dlsym(gLib, "g_free");

        if (!(createUriFunc && nativeFunc && getFunc && freeFunc && initFunc && unrefFunc))
        {
            std::cerr << "Failed to obtain functions from gio libraries" << std::endl;
            return;
        }

        initFunc();

        void* pFile = createUriFunc(filename.c_str());
        if (!pFile)
        {
            std::cerr << "Failed to create gio file: " << filename << std::endl;
            return;
        }

        if (!nativeFunc(pFile))
        {
            unrefFunc(pFile);
            std::cerr << "Not a native file, thumbnailing will likely fail: " << filename << std::endl;
            return;
        }

        char* pPath = getFunc(pFile);
        if (pPath)
        {
            filename = pPath;
            freeFunc(pPath);
        }
        else
        {
            std::cerr << "Failed to get path: " << filename << std::endl;
        }

        unrefFunc(pFile);
    }
}
#endif

ThumbnailerImageType determineImageTypeFromFilename(const std::string& filename)
{
    std::string lowercaseFilename = filename;
    StringOperations::lowercase(lowercaseFilename);

    size_t size = lowercaseFilename.size();
    if ((lowercaseFilename.substr(size - 5, size) == ".jpeg") || (lowercaseFilename.substr(size - 4, size) == ".jpg"))
    {
        return Jpeg;
    }

    return Png;
}
