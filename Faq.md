Change the command value of the GConf key for each mimetype you want to assign to ffmpegthumbnailer. The keys are located in: `/desktop/gnome/thumbnailers/` . So to use ffmpegthumbnailer to create thumbnails for avi files change the command value of
`/desktop/gnome/thumbnailers/video@x-avi` to:

```
/usr/bin/ffmpegthumbnailer -s %s -i %i -o %o -c png -f -t 10
```


  * **How can I use ffmpegthumbnailer in Thunar?**
To use ffmpegthumbnailer in combination with thunar, just install the thunar-thumbnailer packages from your distribution or from http://goodies.xfce.org/projects/thunar-plugins/thunar-thumbnailers. This will properly register ffmpegthumbnailer with thunar.


  * **How can I use ffmpegthumbnailer in KDE?**
Install the kffmpegthumbnailer package available from this website.