project(kffmpegthumbnailer)

find_package(KDE4 REQUIRED)
find_package(PkgConfig)

include(KDE4Defaults)
include(MacroOptionalAddSubdirectory)
include(FindPkgConfig)
include(CPack)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "kffmpegthumbnailer")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")

pkg_check_modules(AVUTIL libavutil)
pkg_check_modules(AVFORMAT libavformat)
pkg_check_modules(AVCODEC libavcodec)
pkg_check_modules(SWSCALE libswscale)
pkg_check_modules(FFMPEG_THUMBNAILER libffmpegthumbnailer)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${AVUTIL_INCLUDES} ${AVFORMAT_INCLUDES} ${AVCODEC_INCLUDES} ${SWSCALE_INCLUDES} ${FFMPEG_THUMBNAILER_INCLUDES})

set(kffmpegthumbnailer_SRCS kffmpegthumbnailer.cpp)
STRING(REGEX REPLACE "-fno-exceptions " "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

kde4_add_plugin(kffmpegthumbnailer ${kffmpegthumbnailer_SRCS})
target_link_libraries(kffmpegthumbnailer ${KDE4_KIO_LIBS} ${AVUTIL_LIBRARIES} ${AVFORMAT_LIBRARIES} ${AVCODEC_LIBRARIES} ${SWSCALE_LIBRARIES} ${FFMPEG_THUMBNAILER_LIBRARIES})

install(TARGETS kffmpegthumbnailer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kffmpegthumbnailer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
