AC_INIT([ffmpegthumbnailer], [2.0.1])
AC_CONFIG_SRCDIR([main.cpp])
AM_INIT_AUTOMAKE

AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_SYS_LARGEFILE

AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER(config.h)

AM_SILENT_RULES

AC_HEADER_STDC
AC_CHECK_HEADERS(
    [inttypes.h sys/stat.h],,
    [AC_MSG_ERROR([Missing headers required to compile ffmpegthumbnailer])]
)

PKG_CHECK_MODULES(  FFMPEG, libavutil libavformat libavcodec libswscale, HAVE_FFMPEG=yes,
                    AC_MSG_ERROR([ +Could not find ffmpeg (libavutil libavformat libavcodec libswscale) ])
)

#check if old style include paths for ffmpeg are used on the system
AC_CHECK_HEADERS(ffmpeg/avcodec.h)
AC_CHECK_HEADERS(ffmpeg/avformat.h)
AC_CHECK_HEADERS(ffmpeg/avutil.h)
AC_CHECK_HEADERS(ffmpeg/swscale.h)


#PKG_CHECK_MODULES( PNG, libpng, HAVE_PNG=yes, AC_MSG_ERROR([ +Could not find libpng]) )

AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LIBS)
#AC_SUBST(PNG_LIBS)

AC_ARG_WITH(pkgconfigdir,
    AC_HELP_STRING([--with-pkgconfigdir],
    [Use the specified pkgconfig dir (default is libdir/pkgconfig)]),
    [pkgconfigdir=${withval}],
    [pkgconfigdir='${libdir}/pkgconfig'])


########################################################################
# Check png support
########################################################################
AM_CONDITIONAL(PNG, test "$HAVE_PNG" = "yes")
AC_ARG_ENABLE(png,
    [  --enable-png            Support png output format],
    [ enable_png=$enableval ] )

if test "$enable_png" != "no"; then
    PKG_CHECK_MODULES(PNG, libpng, HAVE_PNG=yes, HAVE_PNG=no)
    if test $HAVE_PNG = yes; then
        AC_DEFINE(HAVE_PNG, [], "Png output format enabled")
        AC_SUBST(PNG_CFLAGS)
        AC_SUBST(PNG_LIBS)
    fi
fi
AM_CONDITIONAL(PNG, test "$HAVE_PNG" = "yes")

########################################################################
# Check jpeg support
########################################################################
AM_CONDITIONAL(JPEG, test "$HAVE_JPEG" = "yes")
AC_ARG_ENABLE(jpeg,
    [  --enable-jpeg           Support jpeg output format],
    [ enable_jpeg=$enableval ] )

if test "$enable_jpeg" != "no"; then
    AC_CHECK_HEADERS(jpeglib.h, HAVE_JPEG=yes, HAVE_JPEG=no)
    if test $HAVE_JPEG = yes; then
        AC_DEFINE(HAVE_JPEG, [], "Jpeg output format enabled")
        JPEG_LIBS=-ljpeg
        AC_SUBST(JPEG_LIBS)
    fi
fi
AM_CONDITIONAL(JPEG, test "$HAVE_JPEG" = "yes")

########################################################################
# Check debug mode
########################################################################
AC_ARG_ENABLE(debug,
   [  --enable-debug          Build in debug mode ],
   DEBUG=$enableval)

AM_CONDITIONAL(DEBUG, test "$enable_debug" = "yes")
if test "$DEBUG" = "yes"; then
    AC_DEFINE(ENABLE_DEBUG, [], "Enable debug mode")
    CXXFLAGS="-g -O1 -Wall -Werror -Wfatal-errors"
fi

AC_ARG_ENABLE(unittests,
   [  --enable-unittests      Enables build of unittests],
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

if test "x$HAVE_PNG" = xyes; then
    echo "png support         : enabled"
else
    echo "png support         : disabled"
fi

if test "x$HAVE_JPEG" = xyes; then
    echo "jpeg support        : enabled"
else
    echo "jpeg support        : disabled"
fi

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


