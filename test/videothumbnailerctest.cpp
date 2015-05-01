#include <algorithm>

#include <vector>
#include <fstream>
#include <gtest/gtest.h>

#include "libffmpegthumbnailer/videothumbnailerc.h"

using namespace std;

namespace ffmpegthumbnailer
{

class VideoThumbnailercTest : public testing::Test
{
    protected:

    virtual void SetUp() override
    {
        thumbnailer = video_thumbnailer_create();
        imageData = video_thumbnailer_create_image_data();
    }

    virtual void TearDown() override
    {
        video_thumbnailer_destroy_image_data(imageData);
        video_thumbnailer_destroy(thumbnailer);
    }

    image_data* imageData = nullptr;
    video_thumbnailer* thumbnailer = nullptr;
};

TEST_F(VideoThumbnailercTest, DISABLED_CreateThumb)
{
    thumbnailer->seek_percentage        = 15;
    thumbnailer->overlay_film_strip     = 1;
    thumbnailer->thumbnail_size         = 256;
    thumbnailer->thumbnail_image_type   = Jpeg;

    std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";
    EXPECT_EQ(0, video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, input.c_str(), imageData));

    ASSERT_NE(0, imageData->image_data_size);
    ASSERT_NE(nullptr, imageData->image_data_ptr);
}

TEST_F(VideoThumbnailercTest, CreateThumbInvalidFile)
{
    video_thumbnailer_set_log_callback(thumbnailer, [] (ThumbnailerLogLevel, const char* msg) {
        EXPECT_STREQ("Could not open input file: invalidfile.mpg", msg);
    });

    EXPECT_EQ(-1, video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, "invalidfile.mpg", imageData));

    video_thumbnailer_set_log_callback(thumbnailer, nullptr);
    EXPECT_EQ(-1, video_thumbnailer_generate_thumbnail_to_buffer(thumbnailer, "invalidfile.mpg", imageData));
}

}
