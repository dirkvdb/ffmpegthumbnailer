# Introduction #

FFMpeg Thumbnailer is a lightweight video thumbnailer that can be used by file managers.
The thumbnailer is based on ffmpeg so it supports al the video formats that ffmpeg supports.

# Usage #
ffmpegthumbnailer is called from the command line

Available command line arguments:
  * -i: input video filename
  * -o: output filename of the generated image file (filename ending with .jpg or .jpeg will be in jpeg format, otherwise png is used)
  * -s: size of the generated thumbnail in pixels (use 0 for original size) (default value: 128)
  * -t: time to seek to (percentage or absolute time hh:mm:ss) (default: 10)
  * -f: use this option to overlay a movie strip on the generated thumbnail
  * -q: image quality (0 = bad, 10 = best) (default: 8) only applies to jpeg output
  * -c: override image format (jpeg or png) (default: determined by filename)
  * -a: ignore aspect ratio and generate square thumbnail
  * -w: workaround some issues in older versions of ffmpeg (only use if you experience problems like 100% cpu usage on certain files)
  * -h: display the usage