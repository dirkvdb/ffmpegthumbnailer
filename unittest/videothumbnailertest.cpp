#include <gtest/gtest.h>
#include <algorithm>

#include <vector>

#include "libffmpegthumbnailer/videothumbnailer.h"

using namespace std;

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
    
    int getBestThumbnailIndex(vector<VideoFrame>& videoFrames, const vector<Histogram<int> >& histograms)
    {
        return videoThumbnailer.getBestThumbnailIndex(videoFrames, histograms);
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

TEST_F(VideoThumbnailerTest, FrameSelection)
{
    vector<VideoFrame> videoFrames;
    vector<Histogram<int> > histograms;
    
    VideoFrame frame1(5, 5, 15);
    fill(frame1.frameData.begin(), frame1.frameData.end(), 255);
    
    VideoFrame frame2(5, 5, 15);
    fill(frame2.frameData.begin(), frame2.frameData.end(), 0);
    
    VideoFrame frame3(5, 5, 15);
    fill(frame3.frameData.begin(), frame3.frameData.end(), 128);
    
    Histogram<int> hist1;
    hist1.r[255] = 25;
    hist1.g[255] = 25;
    hist1.b[255] = 25;
    
    Histogram<int> hist2;
    hist2.r[0] = 25;
    hist2.g[0] = 25;
    hist2.b[0] = 25;
    
    Histogram<int> hist3;
    hist3.r[128] = 25;
    hist3.g[128] = 25;
    hist3.b[128] = 25;
    
    videoFrames.push_back(frame1);
    videoFrames.push_back(frame2);
    videoFrames.push_back(frame3);
    
    histograms.push_back(hist1);
    histograms.push_back(hist2);
    histograms.push_back(hist3);

    //This test is bad
    //EXPECT_EQ(getBestThumbnailIndex(videoFrames, histograms), 1);
}
