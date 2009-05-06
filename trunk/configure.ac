AC_INIT([ffmpegthumbnailer], [1.5.0])
AC_CONFIG_SRCDIR([main.cpp])
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_SYS_LARGEFILE 

AC_CONFIG_MACRO_DIR([m4])

AC_HEADER_STDC
AC_CHECK_HEADERS(
    [inttypes.h sys/stat.h jpeglib.h],,
    [AC_MSG_ERROR([Missing headers required to compile ffmpegthumbnailer])]
)

PKG_CHECK_MODULES(	FFMPEG, libavutil libavformat libavcodec libswscale, HAVE_FFMPEG=yes, 
					AC_MSG_ERROR([ +Could not find ffmpeg. Please update PKG_CONFIG_PATH to point at location of ffmpeg pkgconfig files directory.])
)

AC_CHECK_HEADERS(ffmpeg/avcodec.h libavcodec/avcodec.h)
AC_CHECK_HEADERS(ffmpeg/avformat.h libavformat/avformat.h)
AC_CHECK_HEADERS(ffmpeg/avutil.h libavutil/avutil.h)
AC_CHECK_HEADERS([ffmpeg/swscale.h libswscale/swscale.h])


PKG_CHECK_MODULES(	PNG, libpng, HAVE_PNG=yes, AC_MSG_ERROR([ +Could not find libpng])
)

AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LIBS)
AC_SUBST(PNG_LIBS)

AC_ARG_WITH(pkgconfigdir,
	AC_HELP_STRING([--with-pkgconfigdir],
	[Use the specified pkgconfig dir (default is libdir/pkgconfig)]),
	[pkgconfigdir=${withval}],
	[pkgconfigdir='${libdir}/pkgconfig'])
	
AC_ARG_ENABLE(debug,
   [  --enable-debug Build in debug mode ],
   DEBUG=$enableval)
   
AM_CONDITIONAL(DEBUG, test "$enable_debug" = "yes")

if test "$DEBUG" = "yes"; then
    AC_DEFINE(DEBUG_MODE)
    CXXFLAGS="$CXXFLAGS -g -Wall -Werror -Wfatal-errors"
fi


AC_ARG_ENABLE(unittests,
   [  --enable-unittests Enables build of unittests ],
   ENABLE_UNITTEST=$enableval)

if test "$ENABLE_UNITTEST" = "yes"; then
    AC_CHECK_HEADERS(
    [gtest/gtest.h],,
    [AC_MSG_ERROR([Missing gtest library, install the google unittest framework])])
    
    UNITTEST_LIBS="$(gtest-config --libs)"
    AC_SUBST(UNITTEST_LIBS)
fi
AM_CONDITIONAL(ENABLE_UNITTEST, test "$enable_unittests" = "yes")

AC_SUBST([pkgconfigdir])
AC_MSG_NOTICE([pkgconfig directory is ${pkgconfigdir}])

AC_CONFIG_FILES([libffmpegthumbnailer.pc:libffmpegthumbnailer.pc.in])

AC_OUTPUT([
	Makefile
	man/Makefile
])


echo
echo "CONFIGURATION SUMMARY ----"

if test "x$ENABLE_UNITTEST" = xyes; then
    echo "unittests           : enabled"
else
    echo "unittests           : disabled"
fi

if test "x$DEBUG" = xyes; then
    echo "debug mode          : enabled"
else
    echo "debug mode          : disabled"
fi

