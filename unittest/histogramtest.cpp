#include <unittest++/UnitTest++.h>
#include <string>
#include <vector>
#include <iostream>

#include "libffmpegthumbnailer/histogram.h"

SUITE(HistogramTest)
{
    TEST(TestCreationInt)
    {
        Histogram<int> hist;
        
        for (int i = 0; i < 255; ++i)
        {
            CHECK_EQUAL(0, hist.r[i]);
            CHECK_EQUAL(0, hist.g[i]);
            CHECK_EQUAL(0, hist.b[i]);
        }
    }
    
    TEST(TestCreationFloat)
    {
        Histogram<float> hist;
        
        for (int i = 0; i < 255; ++i)
        {
            CHECK_EQUAL(0.0, hist.r[i]);
            CHECK_EQUAL(0.0, hist.g[i]);
            CHECK_EQUAL(0.0, hist.b[i]);
        }
    }
}
