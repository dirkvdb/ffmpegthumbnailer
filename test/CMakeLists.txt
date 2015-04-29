INCLUDE_DIRECTORIES(SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}
)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

LINK_DIRECTORIES(
	${XDGBASEDIR_LIBRARY_DIRS}
)

ADD_EXECUTABLE(ffmpegthumbnailertest
    gmock-gtest-all.cpp
    testrunner.cpp
    histogramtest.cpp
    videothumbnailertest.cpp
    videothumbnailerctest.cpp
)

ADD_DEFINITIONS(-DTEST_DATADIR="${CMAKE_CURRENT_SOURCE_DIR}")

IF (ENABLE_SHARED)
    TARGET_LINK_LIBRARIES(ffmpegthumbnailertest ${SHARED_LIB})
ELSE ()
    TARGET_LINK_LIBRARIES(ffmpegthumbnailertest ${STATIC_LIB})
ENDIF ()

IF (UNIX)
    TARGET_LINK_LIBRARIES(ffmpegthumbnailertest -lpthread)
ENDIF ()

ADD_TEST(NAME ffmpegthumbnailertest COMMAND ffmpegthumbnailertest)
