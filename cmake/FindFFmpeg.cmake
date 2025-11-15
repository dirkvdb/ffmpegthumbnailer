include(FindPackageHandleStandardArgs)

find_package(PkgConfig)
pkg_check_modules(AVCODEC libavcodec)
pkg_check_modules(AVFORMAT libavformat)
pkg_check_modules(AVUTIL libavutil)
pkg_check_modules(AVFILTER libavfilter)

find_package_handle_standard_args(FFmpeg
    FOUND_VAR FFmpeg_FOUND
    REQUIRED_VARS AVCODEC_FOUND AVFORMAT_FOUND AVUTIL_FOUND AVFILTER_FOUND
)

FIND_LIBRARY(AVCODEC_LIBRARY_PATH
    NAMES ${AVCODEC_LIBRARIES}
    PATHS ${AVCODEC_LIBRARY_DIRS}
)
if (NOT AVCODEC_LIBRARY_PATH)
    message(FATAL_ERROR "Could not determine full avcodec library path")
endif ()

FIND_LIBRARY(AVFORMAT_LIBRARY_PATH
    NAMES ${AVFORMAT_LIBRARIES}
    PATHS ${AVFORMAT_LIBRARY_DIRS}
)
if (NOT AVFORMAT_LIBRARY_PATH)
    message(FATAL_ERROR "Could not determine full avformat library path")
endif ()

FIND_LIBRARY(AVFILTER_LIBRARY_PATH
    NAMES ${AVFILTER_LIBRARIES}
    PATHS ${AVFILTER_LIBRARY_DIRS}
)
if (NOT AVFILTER_LIBRARY_PATH)
    message(FATAL_ERROR "Could not determine full avfilter library path")
endif ()

FIND_LIBRARY(AVUTIL_LIBRARY_PATH
    NAMES ${AVUTIL_LIBRARIES}
    PATHS ${AVUTIL_LIBRARY_DIRS}
)
if (NOT AVUTIL_LIBRARY_PATH)
    message(FATAL_ERROR "Could not determine full avutil library path")
endif ()

mark_as_advanced(
    AVFORMAT_LIBRARY_PATH
    AVUTIL_LIBRARY_PATH
    AVCODEC_LIBRARY_PATH
    AVFILTER_LIBRARY_PATH
)

# Determine if we need static linking
set(FFMPEG_USE_STATIC_LIBS OFF)
if(NOT BUILD_SHARED_LIBS OR ENABLE_STATIC)
    set(FFMPEG_USE_STATIC_LIBS ON)
endif()

message(STATUS "FFmpeg found: ${FFmpeg_FOUND}")
message(STATUS "  avcodec:  ${AVCODEC_LIBRARY_PATH}")
message(STATUS "  avformat: ${AVFORMAT_LIBRARY_PATH}")
message(STATUS "  avfilter: ${AVFILTER_LIBRARY_PATH}")
message(STATUS "  avutil:   ${AVUTIL_LIBRARY_PATH}")
message(STATUS "  static:   ${FFMPEG_USE_STATIC_LIBS}")

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avformat)
    add_library(FFmpeg::avformat UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avformat PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${AVFORMAT_INCLUDE_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS __STDC_CONSTANT_MACROS
        INTERFACE_COMPILE_OPTIONS "${AVFORMAT_CFLAGS_OTHER}"
        IMPORTED_LOCATION ${AVFORMAT_LIBRARY_PATH}
    )

    # Use _STATIC_LIBRARIES and _STATIC_LIBRARY_DIRS for static builds
    if(FFMPEG_USE_STATIC_LIBS)
        set_property(TARGET FFmpeg::avformat APPEND PROPERTY
            INTERFACE_LINK_DIRECTORIES "${AVFORMAT_STATIC_LIBRARY_DIRS}")
        set_property(TARGET FFmpeg::avformat APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "${AVFORMAT_STATIC_LIBRARIES}")
    endif()
endif()

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avutil)
    add_library(FFmpeg::avutil UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avutil PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${AVUTIL_INCLUDE_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS __STDC_CONSTANT_MACROS
        INTERFACE_COMPILE_OPTIONS "${AVUTIL_CFLAGS_OTHER}"
        IMPORTED_LOCATION ${AVUTIL_LIBRARY_PATH}
    )

    if(FFMPEG_USE_STATIC_LIBS)
        set_property(TARGET FFmpeg::avutil APPEND PROPERTY
            INTERFACE_LINK_DIRECTORIES "${AVUTIL_STATIC_LIBRARY_DIRS}")
        set_property(TARGET FFmpeg::avutil APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "${AVUTIL_STATIC_LIBRARIES}")
    endif()
endif()

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avcodec)
    add_library(FFmpeg::avcodec UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avcodec PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${AVCODEC_INCLUDE_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS __STDC_CONSTANT_MACROS
        INTERFACE_COMPILE_OPTIONS "${AVCODEC_CFLAGS_OTHER}"
        IMPORTED_LOCATION ${AVCODEC_LIBRARY_PATH}
    )

    if(FFMPEG_USE_STATIC_LIBS)
        set_property(TARGET FFmpeg::avcodec APPEND PROPERTY
            INTERFACE_LINK_DIRECTORIES "${AVCODEC_STATIC_LIBRARY_DIRS}")
        set_property(TARGET FFmpeg::avcodec APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "${AVCODEC_STATIC_LIBRARIES}")
    endif()
endif()

if(FFmpeg_FOUND AND NOT TARGET FFmpeg::avfilter)
    add_library(FFmpeg::avfilter UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::avfilter PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${AVFILTER_INCLUDE_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS __STDC_CONSTANT_MACROS
        INTERFACE_COMPILE_OPTIONS "${AVFILTER_CFLAGS_OTHER}"
        IMPORTED_LOCATION ${AVFILTER_LIBRARY_PATH}
    )

    if(FFMPEG_USE_STATIC_LIBS)
        set_property(TARGET FFmpeg::avfilter APPEND PROPERTY
            INTERFACE_LINK_DIRECTORIES "${AVFILTER_STATIC_LIBRARY_DIRS}")
        set_property(TARGET FFmpeg::avfilter APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "${AVFILTER_STATIC_LIBRARIES}")
    endif()
endif()

find_package(Threads)
set_property(TARGET FFmpeg::avutil APPEND PROPERTY INTERFACE_LINK_LIBRARIES Threads::Threads)
