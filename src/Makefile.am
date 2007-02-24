bin_PROGRAMS = ffmpegthumbnailer

ffmpegthumbnailer_SOURCES = main.cpp \
							exception.h \
							videothumbnailer.h videothumbnailer.cpp \
							moviedecoder.h moviedecoder.cpp \
							pngwriter.h pngwriter.cpp \
							stringoperations.h stringoperations.cpp

ffmpegthumbnailer_LDADD = -lavutil -lavcodec -lavformat -lpng
