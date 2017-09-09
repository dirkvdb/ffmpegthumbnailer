#include "config.h"
#include "libffmpegthumbnailer/videothumbnailerc.h"

#include <algorithm>
#include <vector>
#include <fstream>
#include <string.h>
#include <catch.hpp>

using namespace std;

namespace ffmpegthumbnailer
{

TEST_CASE("C API Usage")
{
    auto* thumbnailer = video_thumbnailer_create();
    auto* imageData = video_thumbnailer_create_image_data();

#ifdef HAVE_JPEG
    SECTION("CreateThumbJpeg")
    {
        thumbnailer->seek_percentage        = 15;
        thumbnailer->overlay_film_strip     = 1;
        thumbnailer->thumbnail_size         = 256;
        thumbnailer->thumbnail_image_type   = Jpeg;

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        REQUIRE(0 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

        CHECK(0 != imageData->image_data_size);
        CHECK(nullptr != imageData->image_data_ptr);
    }
#endif

#ifdef HAVE_PNG
    SECTION("CreateThumbPng")
    {
        thumbnailer->seek_percentage = 15;
        thumbnailer->overlay_film_strip = 1;
        thumbnailer->thumbnail_size = 256;
        thumbnailer->thumbnail_image_type = Png;

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        REQUIRE(0 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

        CHECK(0 != imageData->image_data_size);
        CHECK(nullptr != imageData->image_data_ptr);
    }
#endif

    SECTION("CreateThumbInvalidFile")
    {
        video_thumbnailer_set_log_callback(thumbnailer, [] (ThumbnailerLogLevel, const char* msg) {
            CHECK(0 == strcmp("Could not open input file: invalidfile.mpg", msg));
        });

        CHECK(-1 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, "invalidfile.mpg", imageData));

        video_thumbnailer_set_log_callback(thumbnailer, nullptr);
        CHECK(-1 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, "invalidfile.mpg", imageData));
    }

    video_thumbnailer_destroy_image_data(imageData);
    video_thumbnailer_destroy(thumbnailer);
}

}
