AC_INIT([ffmpegthumbnailer], [2.0.4])
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

AC_ARG_WITH(pkgconfigdir,
    AC_HELP_STRING([--with-pkgconfigdir],
    [Use the specified pkgconfig dir (default is libdir/pkgconfig)]),
    [pkgconfigdir=${withval}],
    [pkgconfigdir='${libdir}/pkgconfig'])

#########################################################################
# Enable -Wl,--as-needed with GNU ld to prevent overlinking of libraries
#########################################################################
AC_ARG_ENABLE([as-needed],
  AC_HELP_STRING([--disable-as-needed],
  [Disable overlinking protection]),
  [enable_as_needed=$enableval], [enable_as_needed=yes])

if test "x$enable_as_needed" != "xno"; then
  AC_MSG_CHECKING([whether $LD accepts --as-needed])
  case `$LD --as-needed -v 2>&1 </dev/null` in
  *GNU* | *'with BFD'*)
    LDFLAGS="$LDFLAGS -Wl,--as-needed"
    AC_MSG_RESULT([yes])
    ;;
  *)
    AC_MSG_RESULT([no])
    ;;
  esac
fi

#########################################################################
# Check ffmpeg libraries
#########################################################################
PKG_CHECK_MODULES(  FFMPEG, libavutil libavformat libavcodec libswscale, HAVE_FFMPEG=yes,
                    AC_MSG_ERROR([ +Could not find ffmpeg (libavutil libavformat libavcodec libswscale) ])
)
AC_CHECK_LIB(avcodec, avcodec_decode_video2,, AC_MSG_ERROR([ +Your version of libavcodec is too old ]))
AC_SUBST(FFMPEG_CFLAGS)
AC_SUBST(FFMPEG_LIBS)

########################################################################
# Check png support
########################################################################
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
# Check gnome-vfs support
########################################################################
AC_ARG_ENABLE(gnome-vfs,
    [  --enable-gnome-vfs      Support for gnome-vfs file uris],
    [ ENABLE_GNOME_VFS=$enableval ] )

if test "$ENABLE_GNOME_VFS" = "yes"; then
    AC_CHECK_LIB([dl], [dlopen], DL_LIBS="-ldl")
    AC_SUBST(DL_LIBS)
    AC_DEFINE(HAVE_GNOME_VFS, [], "Gnome-vfs file uri support enabled")
fi

########################################################################
# Compiler flags
########################################################################
CXXFLAGS=-D__STDC_CONSTANT_MACROS

########################################################################
# Check debug mode
########################################################################
AC_ARG_ENABLE(debug,
   [  --enable-debug          Build in debug mode ],
   DEBUG=$enableval)

AM_CONDITIONAL(DEBUG, test "$enable_debug" = "yes")
if test "$DEBUG" = "yes"; then
    AC_DEFINE(ENABLE_DEBUG, [], "Enable debug mode")
    CXXFLAGS+=" -g -O1 -Wall -Werror -Wfatal-errors"
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

if test "x$ENABLE_GNOME_VFS" = xyes; then
    echo "gnome-vfs support   : enabled"
else
    echo "gnome-vfs support   : disabled"
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


