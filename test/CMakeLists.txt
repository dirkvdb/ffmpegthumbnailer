add_executable(ffmpegthumbnailertest
    testrunner.cpp
    histogramtest.cpp
    videothumbnailertest.cpp
    videothumbnailerctest.cpp
)

target_include_directories(ffmpegthumbnailertest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/testframework
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(ffmpegthumbnailertest PRIVATE TEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}")

if (ENABLE_SHARED)
    target_link_libraries(ffmpegthumbnailertest ${SHARED_LIB})
else ()
    target_link_libraries(ffmpegthumbnailertest ${STATIC_LIB})
endif ()

add_test(NAME ffmpegthumbnailertest COMMAND ffmpegthumbnailertest)
