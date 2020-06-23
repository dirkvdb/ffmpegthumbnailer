cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(PACKAGE "ffmpegthumbnailer")
set(PACKAGE_VERSION_MAJOR 2)
set(PACKAGE_VERSION_MINOR 2)
set(PACKAGE_VERSION_PATCH 2)
set(PACKAGE_VERSION ${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH})

set(CPACK_PACKAGE_NAME ${PACKAGE})
set(CPACK_PACKAGE_VERSION_MAJOR ${PACKAGE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PACKAGE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PACKAGE_VERSION_PATCH})
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_FILE_NAME $CPACK_PACKAGE_NAME)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-bin")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/debug/;/release/;/gfx/;/include/;/lib/;/.git/;/.vscode/;/src/tags;out-.*;/.*[.]sublime-.*;~$;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)

if (APPLE)
    set(MACOSX_RPATH ON)
endif ()

project(${PACKAGE} LANGUAGES C CXX)
include(CPack)
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_SHARED "Build shared library" ON)
option(ENABLE_STATIC "Build static library" OFF)
option(ENABLE_TESTS "Build unit tests" ON)
option(ENABLE_GIO "Support for gio file uris" OFF)
option(ENABLE_THUMBNAILER "Register ffmpegthumbnailer as thumbnailer" OFF)
option(SANITIZE_ADDRESS "Build with address sanitizer (detect invalid memory access and memory leaks)" OFF)

if (SANITIZE_ADDRESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif ()

find_package(FFmpeg REQUIRED)
if (NOT FFmpeg_FOUND)
    message(FATAL_ERROR "BAAM")
endif ()
if (NOT TARGET FFmpeg::avcodec)
    message(FATAL_ERROR "BOOM")
endif ()

find_package(JPEG)
if (JPEG_FOUND)
    set(HAVE_JPEG ON)
endif ()

find_package(PNG)
if (TARGET PNG::PNG)
    set(HAVE_PNG ON)
endif ()

set(LIB_HDRS
    libffmpegthumbnailer/videothumbnailer.h
    libffmpegthumbnailer/videothumbnailerc.h
    libffmpegthumbnailer/imagetypes.h
    libffmpegthumbnailer/ffmpegthumbnailertypes.h
    libffmpegthumbnailer/ifilter.h
    libffmpegthumbnailer/videoframe.h
    libffmpegthumbnailer/filmstripfilter.h
)

add_library(libffmpegthumbnailerobj OBJECT
    libffmpegthumbnailer/moviedecoder.h
    libffmpegthumbnailer/moviedecoder.cpp
    libffmpegthumbnailer/imagewriter.h
    libffmpegthumbnailer/imagewriterfactory.h
    libffmpegthumbnailer/rgbwriter.h
    libffmpegthumbnailer/rgbwriter.cpp
    libffmpegthumbnailer/stringoperations.h
    libffmpegthumbnailer/stringoperations.cpp
    libffmpegthumbnailer/videothumbnailer.cpp
    libffmpegthumbnailer/videothumbnailerc.cpp
    libffmpegthumbnailer/ifilter.h
    libffmpegthumbnailer/videoframe.h
    libffmpegthumbnailer/histogram.h
    libffmpegthumbnailer/grayscalefilter.h
    libffmpegthumbnailer/filmstrip.h
    libffmpegthumbnailer/filmstripfilter.h
    libffmpegthumbnailer/filmstripfilter.cpp
)

target_link_libraries(libffmpegthumbnailerobj
    FFmpeg::avformat
    FFmpeg::avcodec
    FFmpeg::avutil
    FFmpeg::avfilter
    $<$<BOOL:${HAVE_JPEG}>:${JPEG_LIBRARIES}>
    $<$<BOOL:${HAVE_PNG}>:PNG::PNG>
)

# we use our own deprecated struct menbers, so disable the warning about it
set_source_files_properties(libffmpegthumbnailer/videothumbnailerc.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated-declarations)

set_target_properties(libffmpegthumbnailerobj PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(libffmpegthumbnailerobj PRIVATE __STDC_CONSTANT_MACROS _FILE_OFFSET_BITS=64)
target_include_directories(libffmpegthumbnailerobj
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)

if (HAVE_JPEG)
    target_sources(libffmpegthumbnailerobj PRIVATE libffmpegthumbnailer/jpegwriter.h libffmpegthumbnailer/jpegwriter.cpp)
    target_include_directories(libffmpegthumbnailerobj PRIVATE ${JPEG_INCLUDE_DIR})
endif ()

if (HAVE_PNG)
    target_sources(libffmpegthumbnailerobj PRIVATE libffmpegthumbnailer/pngwriter.h libffmpegthumbnailer/pngwriter.cpp)
endif ()

set (FFMPEGTHUMBNAILER_SOVERSION_CURRENT 4)
set (FFMPEGTHUMBNAILER_SOVERSION_REVISION 15)
set (FFMPEGTHUMBNAILER_SOVERSION_AGE 1)

if (ENABLE_STATIC)
    add_library(libffmpegthumbnailerstatic STATIC $<TARGET_OBJECTS:libffmpegthumbnailerobj>)
    target_link_libraries(libffmpegthumbnailerstatic
        libffmpegthumbnailerobj
        $<$<BOOL:${ENABLE_GIO}>:${CMAKE_DL_LIBS}>
    )

    set_target_properties(libffmpegthumbnailerstatic PROPERTIES
        OUTPUT_NAME ffmpegthumbnailer
    )
    target_include_directories(libffmpegthumbnailerstatic
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )   
    set (STATIC_LIB libffmpegthumbnailerstatic)
endif ()

if (ENABLE_SHARED)
    add_library(libffmpegthumbnailer SHARED $<TARGET_OBJECTS:libffmpegthumbnailerobj>)
    target_link_libraries(libffmpegthumbnailer
        libffmpegthumbnailerobj
    )

    set_target_properties(libffmpegthumbnailer PROPERTIES
        OUTPUT_NAME ffmpegthumbnailer
        VERSION ${FFMPEGTHUMBNAILER_SOVERSION_CURRENT}.${FFMPEGTHUMBNAILER_SOVERSION_REVISION}.${FFMPEGTHUMBNAILER_SOVERSION_AGE}
        SOVERSION ${FFMPEGTHUMBNAILER_SOVERSION_CURRENT}
        PUBLIC_HEADER "${LIB_HDRS}"
        MACOSX_RPATH TRUE
    )
    target_include_directories(libffmpegthumbnailer
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
    set (SHARED_LIB libffmpegthumbnailer)
endif ()

ADD_EXECUTABLE(ffmpegthumbnailer main.cpp)
target_include_directories(ffmpegthumbnailer PRIVATE ${CMAKE_BINARY_DIR})

if (ENABLE_GIO)
    find_path(DL_INCLUDE dlfcn.h)
    target_include_directories(ffmpegthumbnailer PRIVATE ${DL_INCLUDE})
    target_link_libraries(ffmpegthumbnailer ${CMAKE_DL_LIBS})
endif ()

if (ENABLE_SHARED)
    target_link_libraries(ffmpegthumbnailer ${SHARED_LIB})
else ()
    target_link_libraries(ffmpegthumbnailer ${STATIC_LIB})
endif ()

install(TARGETS ffmpegthumbnailer ${STATIC_LIB} ${SHARED_LIB}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libffmpegthumbnailer
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/man/ffmpegthumbnailer.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
install(FILES ${CMAKE_BINARY_DIR}/libffmpegthumbnailer.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if (ENABLE_THUMBNAILER)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dist/ffmpegthumbnailer.thumbnailer DESTINATION ${CMAKE_INSTALL_DATADIR}/thumbnailers)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libffmpegthumbnailer.pc.in ${CMAKE_BINARY_DIR}/libffmpegthumbnailer.pc @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake" IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake)

if (ENABLE_TESTS)
    enable_testing()
    ADD_SUBDIRECTORY(test)
endif ()

message(STATUS "")
message(STATUS "CONFIGURATION SUMMARY")

if (HAVE_PNG)
    message(STATUS "png support          : enabled")
else ()
    message(STATUS "png support          : disabled")
endif ()

if (HAVE_JPEG)
    message(STATUS "jpeg support         : enabled")
else ()
    message(STATUS "jpeg support         : disabled")
endif ()

if (ENABLE_GIO)
    message(STATUS "gio support          : enabled")
else ()
    message(STATUS "gio support          : disabled")
endif ()

if (ENABLE_THUMBNAILER)
    message(STATUS "register thumbnailer : enabled")
else ()
    message(STATUS "register thumbnailer : disabled")
endif ()

if (ENABLE_TESTS)
    message(STATUS "unittests            : enabled")
else ()
    message(STATUS "unittests            : disabled")
endif ()

if (ENABLE_SHARED)
    message(STATUS "shared library       : enabled")
else ()
    message(STATUS "shared library       : disabled")
endif ()

if (ENABLE_STATIC)
    message(STATUS "static library       : enabled")
else ()
    message(STATUS "static library       : disabled")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "debug mode           : enabled")
else ()
    message(STATUS "debug mode           : disabled")
endif ()
