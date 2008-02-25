AC_INIT([ffmpegthumbnailer], [1.2.0])
AC_CONFIG_SRCDIR([src/main.cpp])
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_SYS_LARGEFILE 

AC_HEADER_STDC
AC_CHECK_HEADERS(
    [inttypes.h sys/stat.h jpeglib.h],,
    [AC_MSG_ERROR([Missing headers required to compile ffmpegthumbnailer])]
)

PKG_CHECK_MODULES(	FFMPEG, libavutil libavformat libavcodec libswscale, HAVE_FFMPEG=yes, 
					AC_MSG_ERROR([ +Could not find ffmpeg. Please update PKG_CONFIG_PATH to point at location of ffmpeg pkgconfig files directory.])
)

PKG_CHECK_MODULES(	PNG, libpng, HAVE_PNG=yes, AC_MSG_ERROR([ +Could not find libpng])
)

AC_SUBST(FFMPEG_LIBS)
AC_SUBST(PNG_LIBS)

AC_ARG_WITH(pkgconfigdir,
	AC_HELP_STRING([--with-pkgconfigdir],
	[Use the specified pkgconfig dir (default is libdir/pkgconfig)]),
	[pkgconfigdir=${withval}],
	[pkgconfigdir='${libdir}/pkgconfig'])

AC_SUBST([pkgconfigdir])
AC_MSG_NOTICE([pkgconfig directory is ${pkgconfigdir}])

AC_CONFIG_FILES([libffmpegthumbnailer.pc:libffmpegthumbnailer.pc.in])

AC_OUTPUT([
	Makefile
	src/Makefile
    src/libffmpegthumbnailer/Makefile
 ])
