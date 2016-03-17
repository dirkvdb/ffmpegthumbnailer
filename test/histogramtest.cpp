#include <string>
#include <vector>
#include <iostream>

#include <catch.hpp>
#include "libffmpegthumbnailer/histogram.h"

namespace ffmpegthumbnailer
{

TEST_CASE("CreationInt")
{
    Histogram<int> hist;

    for (int i = 0; i < 255; ++i)
    {
        REQUIRE(0 == hist.r[i]);
        REQUIRE(0 == hist.g[i]);
        REQUIRE(0 == hist.b[i]);
    }
}

TEST_CASE("CreationFloat")
{
    Histogram<float> hist;

    for (int i = 0; i < 255; ++i)
    {
        REQUIRE(0.0 == hist.r[i]);
        REQUIRE(0.0 == hist.g[i]);
        REQUIRE(0.0 == hist.b[i]);
    }
}

}
