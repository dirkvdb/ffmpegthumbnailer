This video thumbnailer can be used to create thumbnails for your video files. The thumbnailer uses ffmpeg to decode frames from the video files, so supported videoformats depend on the configuration flags of ffmpeg.

This thumbnailer was designed to be as fast and lightweight as possible. The only dependencies are ffmpeg, libpng and libjpeg.

The project also includes a C/C++ library that can be used by developers to generate thumbnails in their projects

## News ##
### Download location has changed ###
Since google code no longer supports downloads, releases can now be found here:
https://drive.google.com/folderview?id=0B-HAKhnW2w9WQ091bWlDMzFDTmM&usp=sharing#list

### ffmpegthumbnailer version 2.0.9 (June 30, 2014) ###
  * Fixed gio library loading issue
  * Support udp sources (thanks to tchristensen)
  * No longer support older ffmpeg versions

### ffmpegthumbnailer version 2.0.8 (August 25, 2012) ###
  * Better ffmpeg detection
  * Fix build issue with recent ffmpeg
  * Optionally register ffmpegthumbnailer as gnome3 thumbnailer (--enable-thumbnailer)

### ffmpegthumbnailer version 2.0.7 (July 24, 2011) ###
  * Fix build issue with recent gcc
  * Fix build issue with recent ffmpeg

### ffmpegthumbnailer version 2.0.6 (January 6, 2011) ###
  * Fixed memory leak

### ffmpegthumbnailer version 2.0.5 (November 30, 2010) ###
  * Replaced deprecate gnome-vfs with gio (--enable-gio)

### ffmpegthumbnailer version 2.0.4 (August 06, 2010) ###
  * Fixed undefined references when compiling with certain LDFLAGS

### ffmpegthumbnailer version 2.0.3 (August 06, 2010) ###
  * Writing to stdout is now supported
  * Support for gnome-vfs uris added. New configure flag (--enable-gnome-vfs), this puts a runtime dependancy on libgnome-vfs-2.0


### ffmpegthumbnailer version 2.0.2 (May 15, 2010) ###
  * Fixed compilation error against latest ffmpeg
  * Size of the filmstrip overlay is dependent on thumbnail size

### ffmpegthumbnailer version 2.0.1 (April 17, 2010) ###
  * Setting the thumbnail size to 0 will use the original video size (thanks to John Fremlin)
  * Fix for video files containing lots of audio packets before a video packet
  * Fixed libs in pkgconfig file (thanks to magnus.tuominen)

### kffmpegthumbnailer version 1.1.0 (January 6, 2010) ###
  * Updated to ffmpegthumbnailer 2.0.0 interface

### ffmpegthumbnailer version 2.0.0 (January 6, 2010) ###
  * Fixed some issues in package-config file (Thanks to ambrop7)
  * C++ library has been put in a namespace
  * C library functions have been renamed

### kffmpegthumbnailer version 1.0.1 (December 28, 2009) ###
  * Fixed thumbnails not being generated for filenames containing non-ascii characters (Thanks to fedux.c)

### kffmpegthumbnailer version 1.0.0 (December 24, 2009) ###
A new project has been created: kffmpegthumbnailer. It is a thumbnailer for kde based on ffmpegthumbnailer which should provide fast video thumbnails for the kde desktop.

### version 1.5.6 (December 19, 2009) ###
  * Fixed segmentation fault when seek in video file fails
  * Command line option added to specify output format of the image regardless of the filename
  * Fixed memory leak in MovieDecoder (thanks to Ulrich Völkel)

### version 1.5.5 (September 23, 2009) ###
  * Fixed build issue with certain versions of autoconf
  * Fixed build issue with recent versions of gcc
  * Fixed thumbnail generation for ogm files
  * Fixed ffmpegthumbnailer hanging on certain filetypes caused by AVFMT\_FLAG\_GENPTS flag of video decoder

### version 1.5.4 (June 22, 2009) ###
  * Fixed support for large files

### version 1.5.3 (June 07, 2009) ###
  * Memory alignment fix causing segmentation fault in ffmpeg with altivec enabled

### version 1.5.2 (May 24, 2009) ###
  * Fixed soname version

### version 1.5.1 (May 11, 2009) ###
  * Optional compilation of jpeg and png support
  * Fixed ffmpeg CFLAGS issue (causing compilation errors on Fedora)
  * Fixed build error against latest ffmpeg (verified against ffmpeg [revision 18769](https://code.google.com/p/ffmpegthumbnailer/source/detail?r=18769))
  * Fixed missing include files for library usage (thanks to gustavo.boiko)

### Version 1.5.0 (March 10, 2009) ###
  * Aspect ratio bug fix (Thanks to S. Eguchi)
  * Support input files from stdin (will ignore seek time)
  * Fixed build error against latest ffmpeg (verified against ffmpeg release 0.5)

### Version 1.4.0 (Januari 4, 2009) ###
  * Filter support added that allows library users to implement filters that will be applied to the frame data
  * Fixed compilation error with gcc 4.4

### Version 1.3.0 (September 20, 2008) ###
  * Option added to ignore aspect ratio and generate square thumbnails
  * Quality option added for jpeg thumbnails
  * Man page added (thanks to Lionel Le Folgoc)
  * Added option to seek to absolute time in stead of percentage

### Version 1.2.6 has been released (June 30, 2008) ###
  * Fixed seeking in large files due to integer overflow (Thanks to AWaters1)
  * Fixed possible linker error (Thanks to pressh)

### Version 1.2.5 has been released ###
  * Fixed crash when thumbnailing raw avi files

### Version 1.2.4 has been released ###
  * Fixed missing includes when compiling with gcc 4.3 (thanks to Samuli Suominen)

### Version 1.2.3 has been released ###
  * ffmegthumbnailer now compiles against latest ffmpeg revisions (thanks to Alexis Ballier)

### Version 1.2.2 has been released ###
  * Fixed linker errors when using certain ldflags

### Version 1.2.1 has been released ###
  * Fixed compilation error in c interface when compiled with c compiler
  * Error handling added to c interface

### Version 1.2.0 has been released ###
  * Option to output files in jpeg
  * ffmpegthumbnailer can now be accessed as a library (libffmpegthumbnailer) by other applications

### Version 1.1.5 has been released ###
I finally looked at some problems I was still experiencing and managed to fix them.
Changelog:
  * Support for large files (files larger than 2Gb)
  * Seeking in h264 files is enabled again, use -w option from commandline to disable this if you have 100% cpu usage (using older versions of ffmpeg)
  * Give up reading packets from a file after a number of attempts to avoid a hanging ffmpegthumbnailer for certain file types (noticed on h264 and ac3 in vob container)