lib_LTLIBRARIES = libffmpegthumbnailer.la

libffmpegthumbnailer_la_SOURCES =   moviedecoder.h moviedecoder.cpp \
                                    imagewriter.h imagewriterfactory.h\
                                    pngwriter.h pngwriter.cpp \
                                    jpegwriter.h jpegwriter.cpp \
                                    stringoperations.h stringoperations.cpp \
                                    videothumbnailer.cpp videothumbnailerc.cpp \
                                    ifilter.h videoframe.h \
                                    grayscalefilter.h \
                                    filmstripfilter.h filmstripfilter.cpp

libffmpegthumbnailer_include_HEADERS    =   videothumbnailer.h \
                                            videothumbnailerc.h \
                                            imagetypes.h \
                                            ifilter.h \
                                            videoframe.h \
                                            filmstripfilter.h

libffmpegthumbnailer_includedir         = $(includedir)/libffmpegthumbnailer
libffmpegthumbnailer_la_LDFLAGS         = -version-info 3:0:0
libffmpegthumbnailer_la_LIBADD          = $(FFMPEG_LIBS) $(PNG_LIBS) -ljpeg
