#include "libffmpegthumbnailer/videothumbnailer.h"

#include <unittest++/UnitTest++.h>
#include <algorithm>

class VideoThumbnailerTester
{
public:
    void generateHistogram(const VideoFrame& videoFrame, Histogram<int>& histogram)
    {
        thumbnailer.generateHistogram(videoFrame, histogram);
    }
    
    VideoThumbnailer thumbnailer;
};

SUITE(VideoThumbnailerTest)
{
    TEST(TestCreateHistogramBlackFrame)
    {
        Histogram<int> hist;
        VideoFrame frame(10, 10, 30);
        frame.frameData.resize(300, 0);
        
        VideoThumbnailerTester tester;
        tester.generateHistogram(frame, hist); 
        
        CHECK_EQUAL(100, hist.r[0]);
        CHECK_EQUAL(100, hist.g[0]);
        CHECK_EQUAL(100, hist.b[0]);
        
        for (int i = 1; i < 255; ++i)
        {
            CHECK_EQUAL(0, hist.r[i]);
            CHECK_EQUAL(0, hist.g[i]);
            CHECK_EQUAL(0, hist.b[i]);
        }
    }
    
    TEST(TestCreateHistogramWhiteFrame)
    {
        Histogram<int> hist;
        VideoFrame frame(10, 10, 30);
        
        frame.frameData.resize(300, 255);
        
        VideoThumbnailerTester tester;
        tester.generateHistogram(frame, hist); 
        
        CHECK_EQUAL(100, hist.r[255]);
        CHECK_EQUAL(100, hist.g[255]);
        CHECK_EQUAL(100, hist.b[255]);
        
        for (int i = 0; i < 254; ++i)
        {
            CHECK_EQUAL(0, hist.r[i]);
            CHECK_EQUAL(0, hist.g[i]);
            CHECK_EQUAL(0, hist.b[i]);
        }
    }
}
