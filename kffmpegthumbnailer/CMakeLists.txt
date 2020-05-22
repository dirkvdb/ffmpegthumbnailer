project(kffmpegthumbnailer)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FindPkgConfig)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui)
find_package(KF5 REQUIRED COMPONENTS KIO I18n Config)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "kffmpegthumbnailer video thumbnailer for kde")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/out/;/.svn/;*~")

include(CPack)

pkg_check_modules(FFMPEG_THUMBNAILER libffmpegthumbnailer)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDES} ${FFMPEG_THUMBNAILER_INCLUDES})

set(kffmpegthumbnailer_SRCS kffmpegthumbnailer.cpp)
string(REGEX REPLACE "-fno-exceptions " "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

kconfig_add_kcfg_files(kffmpegthumbnailer_SRCS kffmpegthumbnailersettings5.kcfgc)
add_library(kffmpegthumbnailer MODULE ${kffmpegthumbnailer_SRCS})
target_link_libraries(kffmpegthumbnailer Qt5::Core Qt5::Gui KF5::KIOWidgets KF5::KIOCore KF5::ConfigCore KF5::ConfigGui ${FFMPEG_THUMBNAILER_LIBRARIES})

install(FILES kffmpegthumbnailersettings5.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS kffmpegthumbnailer DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kffmpegthumbnailer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
