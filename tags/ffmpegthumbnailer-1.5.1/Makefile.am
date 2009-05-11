SUBDIRS = man
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libffmpegthumbnailer.la

libffmpegthumbnailer_la_SOURCES =   libffmpegthumbnailer/moviedecoder.h \
                                    libffmpegthumbnailer/moviedecoder.cpp \
                                    libffmpegthumbnailer/imagewriter.h \
                                    libffmpegthumbnailer/imagewriterfactory.h \
                                    libffmpegthumbnailer/stringoperations.h \
                                    libffmpegthumbnailer/stringoperations.cpp \
                                    libffmpegthumbnailer/videothumbnailer.cpp \
                                    libffmpegthumbnailer/videothumbnailerc.cpp \
                                    libffmpegthumbnailer/ifilter.h \
                                    libffmpegthumbnailer/videoframe.h \
                                    libffmpegthumbnailer/histogram.h \
                                    libffmpegthumbnailer/grayscalefilter.h \
                                    libffmpegthumbnailer/filmstripfilter.h \
                                    libffmpegthumbnailer/filmstripfilter.cpp

if PNG
libffmpegthumbnailer_la_SOURCES +=  libffmpegthumbnailer/pngwriter.h \
                                    libffmpegthumbnailer/pngwriter.cpp
endif

if JPEG
libffmpegthumbnailer_la_SOURCES +=  libffmpegthumbnailer/jpegwriter.h \
                                    libffmpegthumbnailer/jpegwriter.cpp
endif

nobase_include_HEADERS =    libffmpegthumbnailer/videothumbnailer.h \
                            libffmpegthumbnailer/videothumbnailerc.h \
                            libffmpegthumbnailer/imagetypes.h \
                            libffmpegthumbnailer/ifilter.h \
                            libffmpegthumbnailer/histogram.h \
                            libffmpegthumbnailer/videoframe.h \
                            libffmpegthumbnailer/filmstripfilter.h

libffmpegthumbnailer_includedir         = $(includedir)/libffmpegthumbnailer
libffmpegthumbnailer_la_CXXFLAGS        = $(AM_CXXFLAGS) $(FFMPEG_CFLAGS) $(PNG_CFLAGS)
libffmpegthumbnailer_la_LDFLAGS         = -version-info 3:0:1
libffmpegthumbnailer_la_LIBADD          = $(FFMPEG_LIBS) $(PNG_LIBS) $(JPEG_LIBS)

bin_PROGRAMS = ffmpegthumbnailer

ffmpegthumbnailer_SOURCES = main.cpp
ffmpegthumbnailer_LDADD = libffmpegthumbnailer.la

if ENABLE_UNITTEST
check_PROGRAMS = testrunner
TESTS = testrunner
AM_COLOR_TESTS = always
endif

testrunner_SOURCES =    unittest/testrunner.cpp \
                        unittest/histogramtest.cpp \
                        unittest/videothumbnailertest.cpp

testrunner_CXXFLAGS =   $(AM_CXXFLAGS)
testrunner_LDFLAGS  =   libffmpegthumbnailer.la
testrunner_LDADD    =   $(UNITTEST_LIBS)

pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libffmpegthumbnailer.pc

