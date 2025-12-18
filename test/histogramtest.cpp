#include <iostream>
#include <string>
#include <vector>

#include "libffmpegthumbnailer/histogram.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

namespace ffmpegthumbnailer {

TEST_CASE("CreationInt")
{
    Histogram<int> hist;

    for (int i = 0; i < 255; ++i) {
        REQUIRE(0 == hist.r[i]);
        REQUIRE(0 == hist.g[i]);
        REQUIRE(0 == hist.b[i]);
    }
}

TEST_CASE("CreationFloat")
{
    Histogram<float> hist;

    for (int i = 0; i < 255; ++i) {
        REQUIRE(0.0 == hist.r[i]);
        REQUIRE(0.0 == hist.g[i]);
        REQUIRE(0.0 == hist.b[i]);
    }
}

}
