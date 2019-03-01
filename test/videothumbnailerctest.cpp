#include "config.h"
#include "libffmpegthumbnailer/videothumbnailerc.h"

#include <algorithm>
#include <vector>
#include <fstream>
#include <string.h>
#include <catch.hpp>
#include <iostream>

using namespace std;

namespace ffmpegthumbnailer
{

TEST_CASE("C API Usage")
{
    auto* thumbnailer = video_thumbnailer_create();
    auto* imageData = video_thumbnailer_create_image_data();

    thumbnailer->seek_percentage        = 15;
    thumbnailer->overlay_film_strip     = 1;

    video_thumbnailer_set_log_callback(thumbnailer, [] (ThumbnailerLogLevel lvl, const char* msg) {
        if (lvl == ThumbnailerLogLevelError)
        {
            std::cerr << msg << "\n";
        }
    });

#ifdef HAVE_JPEG
    SECTION("CreateThumbJpeg")
    {

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
        thumbnailer->thumbnail_image_type = Png;

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        REQUIRE(0 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

        CHECK(0 != imageData->image_data_size);
        CHECK(nullptr != imageData->image_data_ptr);
    }
#endif

    SECTION("CreateThumbRgb")
    {
        thumbnailer->thumbnail_image_type = Rgb;

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        REQUIRE(0 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

        CHECK(0 != imageData->image_data_size);
        CHECK(nullptr != imageData->image_data_ptr);
        CHECK(ThumbnailerImageSourceVideoStream == imageData->image_data_source);
    }

    SECTION("CreateThumbRgbStringSizeHeightOnly")
    {
        thumbnailer->thumbnail_image_type = Rgb;
        video_thumbnailer_set_size(thumbnailer, 0, 234);

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        REQUIRE(0 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

        CHECK(0 != imageData->image_data_size);
        CHECK(234 == imageData->image_data_height);
        CHECK(nullptr != imageData->image_data_ptr);
        CHECK(ThumbnailerImageSourceVideoStream == imageData->image_data_source);
    }

    SECTION("CreateThumbRgbOriginalSize")
    {
        thumbnailer->thumbnail_image_type = Rgb;
        video_thumbnailer_set_size(thumbnailer, 0, 0);

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        auto rc = video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData);
        REQUIRE(0 == rc);

        CHECK(9 != imageData->image_data_size);
        CHECK(160 == imageData->image_data_width);
        CHECK(120 == imageData->image_data_height);
        CHECK(nullptr != imageData->image_data_ptr);
        CHECK(ThumbnailerImageSourceVideoStream == imageData->image_data_source);
    }

    SECTION("CreateThumbRgbSetSizeBothWidthHeightMaintainAspect")
    {
        thumbnailer->thumbnail_image_type = Rgb;
        thumbnailer->maintain_aspect_ratio = 1;
        video_thumbnailer_set_size(thumbnailer, 200, 234);

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        REQUIRE(0 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

        CHECK(0 != imageData->image_data_size);
        CHECK(234 >= imageData->image_data_height);
        CHECK(200 == imageData->image_data_width);
        CHECK(nullptr != imageData->image_data_ptr);
        CHECK(ThumbnailerImageSourceVideoStream == imageData->image_data_source);
    }

    SECTION("CreateThumbRgbSetSizeBothWidthHeightDoNotMaintainAspect")
    {
        thumbnailer->thumbnail_image_type = Rgb;
        thumbnailer->maintain_aspect_ratio = 0;
        video_thumbnailer_set_size(thumbnailer, 200, 234);

        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
        REQUIRE(0 == video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

        CHECK(234 * 200 * 3 == imageData->image_data_size);
        CHECK(234 == imageData->image_data_height);
        CHECK(200 == imageData->image_data_width);
        CHECK(nullptr != imageData->image_data_ptr);
        CHECK(ThumbnailerImageSourceVideoStream == imageData->image_data_source);
    }

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
