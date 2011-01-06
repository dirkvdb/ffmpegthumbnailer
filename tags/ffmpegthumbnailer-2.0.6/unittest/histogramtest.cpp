#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <iostream>

#include "libffmpegthumbnailer/histogram.h"

namespace ffmpegthumbnailer
{

TEST(HistogramTest, CreationInt)
{
    Histogram<int> hist;
    
    for (int i = 0; i < 255; ++i)
    {
        EXPECT_EQ(0, hist.r[i]);
        EXPECT_EQ(0, hist.g[i]);
        EXPECT_EQ(0, hist.b[i]);
    }
}

TEST(HistogramTest, CreationFloat)
{
    Histogram<float> hist;
    
    for (int i = 0; i < 255; ++i)
    {
        EXPECT_EQ(0.0, hist.r[i]);
        EXPECT_EQ(0.0, hist.g[i]);
        EXPECT_EQ(0.0, hist.b[i]);
    }
}

}
