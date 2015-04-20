#include <iostream>
#include <clocale>

#include "gtest/gtest.h"

int main(int argc, char **argv) {
    std::cout << "Running ffmpegthumbnailer tests" << std::endl;

    if (!setlocale(LC_CTYPE, "en_US.UTF-8"))
    {
        std::cerr << "Locale not specified. Check LANG, LC_CTYPE, LC_ALL" << std::endl;
        return 1;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
