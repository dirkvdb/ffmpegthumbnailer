#include <algorithm>

#include <vector>
#include <fstream>
#include <iostream>
#include <catch.hpp>

#include "libffmpegthumbnailer/videothumbnailer.h"
#include "libffmpegthumbnailer/imagetypes.h"
#include "libffmpegthumbnailer/histogram.h"
#include "libffmpegthumbnailer/histogramutils.h"

namespace ffmpegthumbnailer
{

TEST_CASE("C++ API Usage")
{
    Histogram<int> histogram;

    VideoFrame frame;
    frame.width = 10;
    frame.height = 10;
    frame.lineSize = 30;
    frame.frameData.resize(300);

    VideoThumbnailer videoThumbnailer;

    videoThumbnailer.setLogCallback([] (ThumbnailerLogLevel, const std::string& msg) {
        std::cout << msg << std::endl;
    });

    SECTION("CreateThumb")
    {
        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";

        std::vector<uint8_t> buffer;
        videoThumbnailer.generateThumbnail(input, Png, buffer);
        CHECK_FALSE(buffer.empty());
    }

    SECTION("CreateThumbStringSize")
    {
        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";

        std::vector<uint8_t> buffer;
        videoThumbnailer.setThumbnailSize("w=128:h=-1");
        auto info = videoThumbnailer.generateThumbnail(input, Png, buffer);
        CHECK_FALSE(buffer.empty());
        CHECK(128 == info.width);
        CHECK(info.height > 0);
    }

    SECTION("CreateThumbRawData")
    {
        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";

        std::vector<uint8_t> buffer;
        videoThumbnailer.generateThumbnail(input, Rgb, buffer);
        CHECK_FALSE(buffer.empty());
    }

    SECTION("SpecifyInvalidSize")
    {
        CHECK_THROWS_AS(videoThumbnailer.setThumbnailSize("w="), std::invalid_argument);
        CHECK_THROWS_AS(videoThumbnailer.setThumbnailSize("h="), std::invalid_argument);
        CHECK_THROWS_AS(videoThumbnailer.setThumbnailSize("w=?"), std::invalid_argument);
        CHECK_THROWS_AS(videoThumbnailer.setThumbnailSize("h=?"), std::invalid_argument);
    }

    SECTION("CreateThumbSpecifyWidth")
    {
        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";

        std::vector<uint8_t> buffer;
        videoThumbnailer.setThumbnailSize("w=64");
        videoThumbnailer.generateThumbnail(input, Png, buffer);
        CHECK_FALSE(buffer.empty());
    }

    SECTION("CreateThumbSpecifyHeight")
    {
        std::string input = std::string(TEST_DATADIR) + "/test_sample.flv";

        std::vector<uint8_t> buffer;
        videoThumbnailer.setThumbnailSize("h=64");
        videoThumbnailer.generateThumbnail(input, Png, buffer);
        CHECK_FALSE(buffer.empty());
    }

    SECTION("CreateThumbNonAscii")
    {
        std::string input = std::string(TEST_DATADIR) + "/test_Кругом_шумел.flv";

        std::vector<uint8_t> buffer;
        videoThumbnailer.generateThumbnail(input, Png, buffer);
        CHECK_FALSE(buffer.empty());
    }

    SECTION("CreateThumbInvalidFile")
    {
        std::vector<uint8_t> buffer;
        CHECK_THROWS_AS(videoThumbnailer.generateThumbnail("invalidfile.mpg", Png, buffer), std::logic_error);
    }

    SECTION("CreateHistogramBlackFrame")
    {
        std::fill(frame.frameData.begin(), frame.frameData.end(), 0);
        utils::generateHistogram(frame, histogram);

        CHECK(100 == histogram.r[0]);
        CHECK(100 == histogram.g[0]);
        CHECK(100 == histogram.b[0]);

        for (int i = 1; i < 255; ++i)
        {
            CHECK(0 == histogram.r[i]);
            CHECK(0 == histogram.g[i]);
            CHECK(0 == histogram.b[i]);
        }
    }

    SECTION("CreateHistogramWhiteFrame")
    {
        std::fill(frame.frameData.begin(), frame.frameData.end(), 255);
        utils::generateHistogram(frame, histogram);

        CHECK(100 == histogram.r[255]);
        CHECK(100 == histogram.g[255]);
        CHECK(100 == histogram.b[255]);

        for (int i = 0; i < 254; ++i)
        {
            CHECK(0 == histogram.r[i]);
            CHECK(0 == histogram.g[i]);
            CHECK(0 == histogram.b[i]);
        }
    }
}

}
