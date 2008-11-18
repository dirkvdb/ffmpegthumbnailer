lib_LTLIBRARIES = libffmpegthumbnailer.la

libffmpegthumbnailer_la_SOURCES =   moviedecoder.h moviedecoder.cpp \
                                    imagewriter.h imagewriterfactory.h\
                                    pngwriter.h pngwriter.cpp \
                                    jpegwriter.h jpegwriter.cpp \
                                    stringoperations.h stringoperations.cpp \
                                    videothumbnailer.cpp videothumbnailerc.cpp \
                                    ifilter.h videoframe.h \
                                    grayscalefilter.h

libffmpegthumbnailer_include_HEADERS    = videothumbnailer.h videothumbnailerc.h imagetypes.h ifilter.h videoframe.h

libffmpegthumbnailer_includedir         = $(includedir)/libffmpegthumbnailer
libffmpegthumbnailer_la_LDFLAGS         = -version-info 2:0:0
libffmpegthumbnailer_la_LIBADD          = $(FFMPEG_LIBS) $(PNG_LIBS) -ljpeg
