find_package(Catch2 CONFIG REQUIRED)

include(CTest)
include(Catch)

add_executable(ffmpegthumbnailertest
    testrunner.cpp
    histogramtest.cpp
    videothumbnailertest.cpp
    videothumbnailerctest.cpp
)

target_include_directories(ffmpegthumbnailertest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/testframework
    ${CMAKE_BINARY_DIR}
)

# test binary built with a c compiler instead of c++ compiler
add_executable(ffmpegthumbnailerctest
    testrunner.c
)

target_include_directories(ffmpegthumbnailerctest PRIVATE
    ${CMAKE_BINARY_DIR}
)

target_compile_definitions(ffmpegthumbnailertest PRIVATE TEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(ffmpegthumbnailerctest PRIVATE TEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}")

if (ENABLE_SHARED)
    if (TARGET Catch2::Catch2WithMain)
        target_link_libraries(ffmpegthumbnailertest libffmpegthumbnailer Catch2::Catch2WithMain)
    else ()
        target_link_libraries(ffmpegthumbnailertest libffmpegthumbnailer Catch2::Catch2)
    endif ()
    target_link_libraries(ffmpegthumbnailerctest libffmpegthumbnailer)
else ()
    if (TARGET Catch2::Catch2WithMain)
        target_link_libraries(ffmpegthumbnailertest libffmpegthumbnailerstatic Catch2::Catch2WithMain)
    else ()
        target_link_libraries(ffmpegthumbnailertest libffmpegthumbnailerstatic Catch2::Catch2)
    endif ()
    target_link_libraries(ffmpegthumbnailerctest libffmpegthumbnailerstatic)
endif ()

catch_discover_tests(ffmpegthumbnailertest)
add_test(NAME ffmpegthumbnailerctest COMMAND ffmpegthumbnailerctest)
