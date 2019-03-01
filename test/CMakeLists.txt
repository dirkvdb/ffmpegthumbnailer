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
    target_link_libraries(ffmpegthumbnailertest libffmpegthumbnailer)
    target_link_libraries(ffmpegthumbnailerctest libffmpegthumbnailer)
else ()
    target_link_libraries(ffmpegthumbnailertest libffmpegthumbnailerstatic)
    target_link_libraries(ffmpegthumbnailerctest libffmpegthumbnailerstatic)
endif ()

add_test(NAME ffmpegthumbnailertest COMMAND ffmpegthumbnailertest)
add_test(NAME ffmpegthumbnailerctest COMMAND ffmpegthumbnailerctest)
