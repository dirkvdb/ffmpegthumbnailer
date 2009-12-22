project(kffmpegthumbnailer)

libfind_pkg_check_modules(FFMPEG_THUMBNAILER libffmpegthumbnailer)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroOptionalAddSubdirectory)

set(kffmpegthumbnailer_SRCS videothumbnail.cpp)

kde4_add_plugin(kffmpegthumbnailer ${videothumbnail_SRCS})
target_link_libraries(kffmpegthumbnailer ${KDE4_KIO_LIBS} $(FFMPEG_THUMBNAILER_LIBS))

install(TARGETS videothumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kffmpegthumbnailer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
