//    Copyright (C) 2024 Dirk Vanden Boer <dirk.vdb@gmail.com>
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
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

extern "C" {
#include <libavutil/log.h>
}

#ifdef ENABLE_GIO
#include <dlfcn.h>
#endif

#include "libffmpegthumbnailer/filmstripfilter.h"
#include "libffmpegthumbnailer/stringoperations.h"
#include "libffmpegthumbnailer/videothumbnailer.h"

using namespace ffmpegthumbnailer;

namespace {

std::string escapeForOutput(std::string_view text)
{
    std::string output;
    output.reserve(text.size());
    for (unsigned char byte : text) {
        // Keep UTF-8 bytes intact; make controls visible, including embedded line breaks.
        if (byte < 0x20 || byte == 0x7F) {
            char escaped[5];
            std::snprintf(escaped, sizeof(escaped), "\\x%02X", static_cast<unsigned>(byte));
            output += escaped;
        } else {
            output += static_cast<char>(byte);
        }
    }
    return output;
}

int formatFFmpegLogLine(void* context, int level, const char* format, va_list args,
                       char* buffer, int bufferSize, int* printPrefix)
{
    va_list copy;
    va_copy(copy, args);
    int length = av_log_format_line2(context, level, format, copy, buffer, bufferSize, printPrefix);
    va_end(copy);
    return length;
}

void ffmpegLogCallback(void* context, int level, const char* format, va_list args)
{
    if (level > av_log_get_level()) {
        return;
    }

    // FFmpeg can log from decoder threads; keep each formatted message together.
    static std::mutex logMutex;
    static thread_local int printPrefix = 1;

    try {
        std::lock_guard<std::mutex> lock(logMutex);
        char buffer[1024];
        int nextPrefix = printPrefix;
        int length = formatFFmpegLogLine(context, level, format, args, buffer, sizeof(buffer), &nextPrefix);
        if (length < 0) {
            return;
        }

        std::vector<char> expanded;
        const char* message = buffer;
        if (static_cast<size_t>(length) >= sizeof(buffer)) {
            expanded.resize(static_cast<size_t>(length) + 1);
            nextPrefix = printPrefix;
            formatFFmpegLogLine(context, level, format, args, expanded.data(), expanded.size(), &nextPrefix);
            message = expanded.data();
        }
        printPrefix = nextPrefix;

        std::fputs(message, stderr);
    } catch (...) {
        // Never propagate C++ exceptions through FFmpeg's C logging callback.
    }
}

}

void printVersion();
void printUsage();
void tryUriConvert(std::string& filename);
ThumbnailerImageType determineImageTypeFromString(const std::string& filename);
ThumbnailerImageType determineImageTypeFromFilename(const std::string& filename);

int main(int argc, char** argv)
{
    av_log_set_callback([](void*, int, const char*, va_list) {});

    int option;
    int seekPercentage = 10;
    std::string seekTime;
    std::string thumbnailSize   = "128";
    int imageQuality            = 8;
    bool filmStripOverlay       = false;
    bool workaroundIssues       = false;
    bool maintainAspectRatio    = true;
    bool smartFrameSelection    = false;
    bool preferEmbeddedMetadata = false;
    std::string inputFile;
    std::string outputFile;
    std::string imageFormat;
    std::string allowedProtocols;
    bool allowedProtocolsSpecified = false;

    if (!std::setlocale(LC_CTYPE, "")) {
        std::cerr << "Failed to set locale" << std::endl;
    }

    while ((option = getopt(argc, argv, "i:o:s:t:q:c:r:P:afwhvVpm")) != -1) {
        switch (option) {
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
            if (std::string(optarg).find(':') != std::string::npos) {
                seekTime = optarg;
            } else {
                try {
                    seekPercentage = std::stoi(optarg);
                } catch (std::invalid_argument&) {
                    std::cerr << "Invalid seek percentage: " << escapeForOutput(optarg) << std::endl;
                    return EXIT_FAILURE;
                }
            }
            break;
        case 'w':
            workaroundIssues = true;
            break;
        case 'q':
            try {
                imageQuality = std::stoi(optarg);
            } catch (std::invalid_argument&) {
                std::cerr << "Invalid image quality: " << escapeForOutput(optarg) << std::endl;
                return EXIT_FAILURE;
            }
            break;
        case 'c':
            imageFormat = optarg;
            break;
        case 'P':
            allowedProtocols = optarg;
            allowedProtocolsSpecified = true;
            break;
        case 'h':
            printUsage();
            return 0;
        case 'V':
            av_log_set_level(AV_LOG_VERBOSE);
            av_log_set_callback(ffmpegLogCallback);
            break;
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

    if (inputFile.empty() || outputFile.empty()) {
        std::cerr << "invalid arguments" << std::endl;
        printUsage();
        return 0;
    }

    if (outputFile == "-" && imageFormat.empty()) {
        std::cerr << "When writing to stdout the image format needs to be specified (e.g.: -c png)" << std::endl;
        return 0;
    }

    try {
#ifdef ENABLE_GIO
        tryUriConvert(inputFile);
#endif
        ThumbnailerImageType imageType = imageFormat.empty() ? determineImageTypeFromFilename(outputFile)
                                                             : determineImageTypeFromString(imageFormat);

        VideoThumbnailer videoThumbnailer(0, workaroundIssues, maintainAspectRatio, imageQuality, smartFrameSelection);
        if (allowedProtocolsSpecified) {
            videoThumbnailer.setAllowedProtocols(allowedProtocols);
        }
        videoThumbnailer.setThumbnailSize(thumbnailSize);
        videoThumbnailer.setLogCallback([](ThumbnailerLogLevel lvl, const std::string& msg) {
            std::cerr << escapeForOutput(msg) << std::endl;
        });

        std::unique_ptr<FilmStripFilter> filmStripFilter;

        if (filmStripOverlay) {
            filmStripFilter.reset(new FilmStripFilter());
            videoThumbnailer.addFilter(filmStripFilter.get());
        }

        videoThumbnailer.setPreferEmbeddedMetadata(preferEmbeddedMetadata);

        if (!seekTime.empty()) {
            std::cout << "Seeking to time: " << escapeForOutput(seekTime) << std::endl;
            videoThumbnailer.setSeekTime(seekTime);
        } else {
            videoThumbnailer.setSeekPercentage(seekPercentage);
        }

        videoThumbnailer.generateThumbnail(inputFile, imageType, outputFile);
    } catch (std::exception& e) {
        std::cerr << "Error: " << escapeForOutput(e.what()) << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unexpected error" << std::endl;
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
              << "  -P<s>   : allowed input protocols, comma-separated (default: ALL; empty denies all)\n"
              << "  -a      : ignore aspect ratio and generate square thumbnail\n"
              << "  -f      : create a movie strip overlay\n"
              //<< "  -p      : use smarter frame selection (slower)\n"
              << "  -m      : prefer embedded image metadata over video content\n"
              << "  -w      : workaround issues in old versions of ffmpeg\n"
              << "  -v      : print version number\n"
              << "  -V      : enable verbose FFmpeg logging to stderr\n"
              << "  -h      : display this help\n";
}

ThumbnailerImageType determineImageTypeFromString(const std::string& type)
{
    std::string lowercaseType = type;
    StringOperations::lowercase(lowercaseType);

    if (lowercaseType == "png") {
        return Png;
    }

    if (lowercaseType == "jpeg" || lowercaseType == "jpg") {
        return Jpeg;
    }

    if (lowercaseType == "raw" || lowercaseType == "rgb") {
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
        if (!m_pLib) std::cerr << escapeForOutput(dlerror()) << std::endl;
    }

    ~LibHandle()
    {
        if (m_pLib) dlclose(m_pLib);
    }

    operator void*() const
    {
        return m_pLib;
    }
    operator bool() const
    {
        return m_pLib != nullptr;
    }

private:
    void* m_pLib;
};

void tryUriConvert(std::string& filename)
{
    if (filename.find(":") == std::string::npos) {
        return;
    }

    LibHandle gLib("libglib-2.0.so.0");
    LibHandle gobjectLib("libgobject-2.0.so.0");
    LibHandle gioLib("libgio-2.0.so.0");

    if (gioLib && gLib && gobjectLib) {
        FileCreateFunc createUriFunc = (FileCreateFunc)dlsym(gioLib, "g_file_new_for_uri");

        IsNativeFunc nativeFunc = (IsNativeFunc)dlsym(gioLib, "g_file_is_native");
        FileGetFunc getFunc     = (FileGetFunc)dlsym(gioLib, "g_file_get_path");
        InitFunc initFunc       = (InitFunc)dlsym(gobjectLib, "g_type_init");
        UnrefFunc unrefFunc     = (UnrefFunc)dlsym(gobjectLib, "g_object_unref");
        FreeFunc freeFunc       = (FreeFunc)dlsym(gLib, "g_free");

        if (!(createUriFunc && nativeFunc && getFunc && freeFunc && initFunc && unrefFunc)) {
            std::cerr << "Failed to obtain functions from gio libraries" << std::endl;
            return;
        }

        initFunc();

        void* pFile = createUriFunc(filename.c_str());
        if (!pFile) {
            std::cerr << "Failed to create gio file: " << escapeForOutput(filename) << std::endl;
            return;
        }

        if (!nativeFunc(pFile)) {
            unrefFunc(pFile);
            std::cerr << "Not a native file, thumbnailing will likely fail: " << escapeForOutput(filename) << std::endl;
            return;
        }

        char* pPath = getFunc(pFile);
        if (pPath) {
            filename = pPath;
            freeFunc(pPath);
        } else {
            std::cerr << "Failed to get path: " << escapeForOutput(filename) << std::endl;
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
    if ((size >= 5 && lowercaseFilename.substr(size - 5, size) == ".jpeg") ||
        (size >= 4 && lowercaseFilename.substr(size - 4, size) == ".jpg")) {
        return Jpeg;
    }

    return Png;
}
