#include <gtest/gtest.h>
#include <algorithm>

#include "libffmpegthumbnailer/videothumbnailer.h"

class VideoThumbnailerTest : public testing::Test
{
    protected:
  
    virtual void SetUp()
    {
        frame.width = 10;
        frame.height = 10;
        frame.lineSize = 30;
        frame.frameData.resize(300);
    }
    
    void generateHistogram()
    {
        videoThumbnailer.generateHistogram(frame, histogram);
    }

    Histogram<int> histogram;
    VideoFrame frame;
    VideoThumbnailer videoThumbnailer;
};

TEST_F(VideoThumbnailerTest, CreateHistogramBlackFrame)
{
    fill(frame.frameData.begin(), frame.frameData.end(), 0);
    generateHistogram();
    
    EXPECT_EQ(100, histogram.r[0]);
    EXPECT_EQ(100, histogram.g[0]);
    EXPECT_EQ(100, histogram.b[0]);
    
    for (int i = 1; i < 255; ++i)
    {
        EXPECT_EQ(0, histogram.r[i]);
        EXPECT_EQ(0, histogram.g[i]);
        EXPECT_EQ(0, histogram.b[i]);
    }
}

TEST_F(VideoThumbnailerTest, CreateHistogramWhiteFrame)
{
    fill(frame.frameData.begin(), frame.frameData.end(), 255);
    generateHistogram();
    
    EXPECT_EQ(100, histogram.r[255]);
    EXPECT_EQ(100, histogram.g[255]);
    EXPECT_EQ(100, histogram.b[255]);
    
    for (int i = 0; i < 254; ++i)
    {
        EXPECT_EQ(0, histogram.r[i]);
        EXPECT_EQ(0, histogram.g[i]);
        EXPECT_EQ(0, histogram.b[i]);
    }
}
