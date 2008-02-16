bin_PROGRAMS = ffmpegthumbnailer

ffmpegthumbnailer_SOURCES = main.cpp \
							videothumbnailer.h videothumbnailer.cpp \
							moviedecoder.h moviedecoder.cpp \
							pngwriter.h pngwriter.cpp \
							stringoperations.h stringoperations.cpp

ffmpegthumbnailer_LDADD = $(FFMPEG_LIBS) $(PNG_LIBS)
