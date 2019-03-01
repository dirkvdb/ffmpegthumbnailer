#include "config.h"
#include "libffmpegthumbnailer/videothumbnailerc.h"

#include <stdio.h>
#include <stdlib.h>

static void log_cb(ThumbnailerLogLevel log_level, const char* msg)
{
    if (log_level == ThumbnailerLogLevelError)
    {
        printf("warn: %s\n", msg);
    }
    else
    {
        printf("info: %s\n", msg);
    }
}

int main(int argc, char** argv)
{
    const char* input = TEST_DATADIR "/test_sample.flv";

    video_thumbnailer* thumbnailer = video_thumbnailer_create();
    image_data* imageData = video_thumbnailer_create_image_data();

    thumbnailer->seek_percentage        = 15;
    thumbnailer->overlay_film_strip     = 1;

    video_thumbnailer_set_log_callback(thumbnailer, log_cb);
    if (0 != video_thumbnailer_set_size(thumbnailer, 256, 0)) {
        return EXIT_FAILURE;
    }

#ifdef HAVE_JPEG
    thumbnailer->thumbnail_image_type   = Jpeg;

    if (0 != video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input, imageData))
    {
        return EXIT_FAILURE;
    }

    if (0 == imageData->image_data_size)
    {
        return EXIT_FAILURE;
    }

    if (NULL == imageData->image_data_ptr)
    {
        return EXIT_FAILURE;
    }
#endif

    return EXIT_SUCCESS;
}
