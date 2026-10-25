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
add_test(NAME clioutputtest
    COMMAND ${CMAKE_COMMAND}
        "-DTHUMBNAILER=$<TARGET_FILE:ffmpegthumbnailer>"
        "-DTEST_WORKDIR=${CMAKE_CURRENT_BINARY_DIR}/clioutputtest"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/clioutputtest.cmake"
)

find_program(FFMPEG_EXECUTABLE NAMES ffmpeg)
if (FFMPEG_EXECUTABLE)
    foreach (encoder mpeg4 libx264 libx265)
        add_test(NAME seektest_${encoder}
            COMMAND ${CMAKE_COMMAND}
                "-DTHUMBNAILER=$<TARGET_FILE:ffmpegthumbnailer>"
                "-DFFMPEG=${FFMPEG_EXECUTABLE}"
                "-DENCODER=${encoder}"
                "-DTEST_WORKDIR=${CMAKE_CURRENT_BINARY_DIR}/seektest_${encoder}"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/seektest.cmake"
        )
        set_tests_properties(seektest_${encoder} PROPERTIES
            TIMEOUT 180
            SKIP_REGULAR_EXPRESSION "SKIP: encoder .* is unavailable"
        )
    endforeach ()
endif ()
