AC_INIT([ffmpegthumbnailer], [1.1.4])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_INSTALL

PKG_CHECK_MODULES(	FFMPEG, libavutil libavformat libavcodec libswscale, HAVE_FFMPEG=yes, 
					AC_MSG_ERROR([ +Could not find ffmpeg. Please update PKG_CONFIG_PATH to point at location of ffmpeg pkgconfig files directory.])
)

PKG_CHECK_MODULES(	PNG, libpng, HAVE_PNG=yes, AC_MSG_ERROR([ +Could not find libpng])
)

AC_SUBST(FFMPEG_LIBS)
AC_SUBST(PNG_LIBS)

AC_OUTPUT([
	Makefile
	src/Makefile
 ])
