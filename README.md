[![Build](https://github.com/dirkvdb/ffmpegthumbnailer/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/dirkvdb/ffmpegthumbnailer/actions/workflows/build.yml)

FFmpegthumbnailer is a lightweight thumbnailer that can be used by file
managers to create thumbnails for videos. It uses ffmpeg to decode video
frames, so supported formats depend on the configuration flags of ffmpeg. It can
also extract embedded album art from audio files to create thumbnails.

This thumbnailer was designed to be as fast and lightweight as possible. The
only dependencies are ffmpeg and libpng/libjpeg.

# Usage #
ffmpegthumbnailer is called from the command line

Available command line arguments:
  * -i: input video filename
  * -o: output filename of the generated image file (filename ending with .jpg or .jpeg will be in jpeg format, otherwise png is used)
  * -s: size of the generated thumbnail in pixels (use 0 for original size) (default value: 128)
  * -t: time to seek to (percentage or absolute time hh:mm:ss) (default: 10%)
  * -f: use this option to overlay a movie strip on the generated thumbnail
  * -q: image quality (0 = bad, 10 = best) (default: 8) only applies to jpeg output
  * -c: override image format (jpeg or png) (default: determined by filename)
  * `-P <protocols>`: override the comma-separated input protocol allowlist (default: ALL); an empty string denies all protocols
  * -a: ignore aspect ratio and generate square thumbnail
  * -m: prefer embedded image metadata over video content
  * -w: workaround some issues in older versions of ffmpeg (only use if you experience problems like 100% cpu usage on certain files)
  * -v: print version number
  * -h: display the usage

## Input protocols and security

All protocols supported by the FFmpeg build are allowed by default (`ALL`).
File managers and other callers can use `-P` to restrict the allowlist. For example,
to allow only local files and pipes:

```sh
ffmpegthumbnailer -i video.mp4 -o thumbnail.png -P file,pipe
```

Omitting `-P` allows all protocols. Passing `-P ALL` also allows all protocols,
while `-P ''` denies all input protocols.
The allowlist also restricts nested protocols, such as those
opened by playlists or used as transports by other protocols.

Network access is enabled by default, so this default does not prevent SSRF. Applications accepting untrusted input should restrict the
allowlist (for example, to `file,pipe`) and apply network isolation as needed.

A caller-provided `AVFormatContext` is the caller's responsibility,
including its I/O and protocol restrictions.

## Credits

<img alt="Static Badge" src="https://img.shields.io/badge/OSS%20hosting%20by-cloudsmith-blue?logo=cloudsmith&style=flat-square&link=https%3A%2F%2Fcloudsmith.com">
</img>

Package repository hosting is graciously provided by [Cloudsmith](https://cloudsmith.com).
Cloudsmith is the only fully hosted, cloud-native, universal package management solution, that
enables your organization to create, store and share packages in any format, to any place, with total
confidence.
