SUBDIRS = man
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libffmpegthumbnailer.la

libffmpegthumbnailer_la_SOURCES =   libffmpegthumbnailer/moviedecoder.h \
                                    libffmpegthumbnailer/moviedecoder.cpp \
                                    libffmpegthumbnailer/imagewriter.h \
                                    libffmpegthumbnailer/imagewriterfactory.h \
                                    libffmpegthumbnailer/pngwriter.h \
                                    libffmpegthumbnailer/pngwriter.cpp \
                                    libffmpegthumbnailer/jpegwriter.h \
                                    libffmpegthumbnailer/jpegwriter.cpp \
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

nobase_include_HEADERS =    libffmpegthumbnailer/videothumbnailer.h \
                            libffmpegthumbnailer/videothumbnailerc.h \
                            libffmpegthumbnailer/imagetypes.h \
                            libffmpegthumbnailer/ifilter.h \
                            libffmpegthumbnailer/videoframe.h \
                            libffmpegthumbnailer/filmstripfilter.h

libffmpegthumbnailer_includedir         = $(includedir)/libffmpegthumbnailer
libffmpegthumbnailer_la_LDFLAGS         = -version-info 3:0:0
libffmpegthumbnailer_la_LIBADD          = $(FFMPEG_LIBS) $(PNG_LIBS) -ljpeg

bin_PROGRAMS = ffmpegthumbnailer

ffmpegthumbnailer_SOURCES = libffmpegthumbnailer/main.cpp
ffmpegthumbnailer_LDADD = libffmpegthumbnailer.la

pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libffmpegthumbnailer.pc

