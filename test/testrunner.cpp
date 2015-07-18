#include <iostream>
#include <clocale>

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    if (!std::setlocale(LC_CTYPE, ""))
    {
        std::cerr << "Failed to set locale" << std::endl;
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
