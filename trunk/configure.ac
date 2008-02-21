AC_INIT([ffmpegthumbnailer], [1.1.5])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_SYS_LARGEFILE 

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
    src/libffmpegthumbnailer/Makefile
 ])
