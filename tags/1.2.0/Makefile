CC = g++
LD = g++
CFLAGS = -c -D NDEBUG -Wall
CFLAGS_DEBUG = -c -g -Wall

TARGET_CFLAGS = $(CFLAGS) 

LIBS = -lavformat -lavcodec -lavutil -lpng

videothumbnailer: thumbnailer.cpp moviedecoder.o videothumbnailer.o pngwriter.o stringoperations.o
	$(LD) $^ $(LIBS) -o $@

moviedecoder.o: moviedecoder.h moviedecoder.cpp
	$(CC) $(TARGET_CFLAGS) moviedecoder.cpp
	
videothumbnailer.o: videothumbnailer.h videothumbnailer.cpp
	$(CC) $(TARGET_CFLAGS) videothumbnailer.cpp
	
pngwriter.o: pngwriter.h pngwriter.cpp
	$(CC) $(TARGET_CFLAGS) pngwriter.cpp
	
stringoperations.o: stringoperations.h stringoperations.cpp
	$(CC) $(TARGET_CFLAGS) stringoperations.cpp
	
clean:
	rm -f *.o videothumbnailer
	
install:
	chmod +x videothumbnailer
	install videothumbnailer /usr/local/bin/videothumbnailer
	install -d /usr/local/share/thumbnailers
	install video-thumbnailer.desktop /usr/share/thumbnailers/video-thumbnailer.desktop
	
uninstall:
	rm -f /usr/local/bin/videothumbnailer
	rm -f /usr/share/thumbnailers/video-thumbnailer.desktop
	