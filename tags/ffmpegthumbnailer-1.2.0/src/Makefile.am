SUBDIRS = libffmpegthumbnailer

bin_PROGRAMS = ffmpegthumbnailer

ffmpegthumbnailer_SOURCES = main.cpp
ffmpegthumbnailer_LDADD =  $(FFMPEG_LIBS) $(PNG_LIBS) \
                            libffmpegthumbnailer/libffmpegthumbnailer.la \
                            -ljpeg
