cmake_minimum_required(VERSION 3.12)

# Issue #220: explicit times seek accurately; percentages retain fast keyframe seeks.
# Generate fixtures at test time; no encoder-specific binary fixtures are needed.
execute_process(
    COMMAND "${FFMPEG}" -hide_banner -encoders
    RESULT_VARIABLE result
    OUTPUT_VARIABLE encoders
    ERROR_VARIABLE stderr
    TIMEOUT 10
)
if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "Could not list FFmpeg encoders (${result}): ${stderr}")
endif()
if(NOT encoders MATCHES "[ \t]${ENCODER}[ \t]")
    message("SKIP: encoder ${ENCODER} is unavailable")
    return()
endif()

file(MAKE_DIRECTORY "${TEST_WORKDIR}")

function(run_checked description)
    execute_process(
        COMMAND ${ARGN}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
        TIMEOUT 30
    )
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${description} failed (${result}):\n${stdout}\n${stderr}")
    endif()
endfunction()

set(encoder_options -g 18 -bf 2)
if(ENCODER STREQUAL "mpeg4")
    list(APPEND encoder_options -q:v 2 -sc_threshold 1000000000)
elseif(ENCODER STREQUAL "libx264")
    list(APPEND encoder_options -preset ultrafast
        -x264-params "keyint=18:min-keyint=18:scenecut=0:bframes=2:b-adapt=0")
elseif(ENCODER STREQUAL "libx265")
    list(APPEND encoder_options -preset ultrafast
        -x265-params "pools=none:frame-threads=1:keyint=18:min-keyint=18:scenecut=0:bframes=2:b-adapt=0:log-level=error")
else()
    message(FATAL_ERROR "Unsupported test encoder: ${ENCODER}")
endif()

# Thirty frames spanning 20 seconds, with keyframes twelve seconds apart.
# The non-integral frame rate also tests first-timestamp >= target (not nearest
# or preceding frame): 1s selects 1.333s and 19s selects the final frame, 19.333s.
# B-frame reordering means reaching that final frame requires decoder draining.
set(input "${TEST_WORKDIR}/long-gop.mp4")
set(offset_input "${TEST_WORKDIR}/long-gop-offset.mp4")
run_checked("Generating ${ENCODER} fixture"
    "${FFMPEG}" -nostdin -hide_banner -loglevel error -y
    -f lavfi -i "testsrc2=size=96x64:rate=3/2:duration=20"
    -an -c:v "${ENCODER}" -threads 1 -pix_fmt yuv420p ${encoder_options}
    "${input}"
)
# Shift the entire presentation timeline without re-encoding. Requested seek
# times must remain relative to AVFormatContext::start_time, now seven seconds.
run_checked("Shifting ${ENCODER} fixture timestamps"
    "${FFMPEG}" -nostdin -hide_banner -loglevel error -y
    -i "${input}" -map 0:v:0 -c copy -output_ts_offset 7 "${offset_input}"
)

set(failures "")
foreach(variant plain offset)
    if(variant STREQUAL "plain")
        set(video "${input}")
    else()
        set(video "${offset_input}")
    endif()
    set(reference_hashes "")
    foreach(second 0 1 4 6 18 19)
        set(reference "${TEST_WORKDIR}/${variant}-${second}-reference.rgb")
        file(REMOVE "${reference}")

        # Do not use input -ss: the oracle must decode sequentially, independent
        # of demuxer seeking. Normalize timestamps for the offset fixture too.
        run_checked("Extracting ${ENCODER}/${variant} reference at ${second}s"
            "${FFMPEG}" -nostdin -hide_banner -loglevel error -y
            -threads 1 -i "${video}" -an -filter_threads 1
            -vf "setpts=PTS-STARTPTS,select=gte(t\\,${second}),scale=w=96:h=64,format=pix_fmts=rgb24"
            -fps_mode passthrough -frames:v 1 -threads 1 -f rawvideo "${reference}"
        )
        file(READ "${reference}" reference_rgb HEX)
        string(LENGTH "${reference_rgb}" reference_length)
        if(NOT reference_length EQUAL 36864) # 96 * 64 * 3 bytes, in hex
            message(FATAL_ERROR "Expected exactly one 96x64 RGB reference frame at ${second}s")
        endif()
        file(SHA256 "${reference}" reference_hash)
        if(reference_hash IN_LIST reference_hashes)
            message(FATAL_ERROR "Fixture must have distinct frames at the requested seek times")
        endif()
        list(APPEND reference_hashes "${reference_hash}")
        if(second EQUAL 0)
            set(first_frame_hash "${reference_hash}")
        endif()

        # Fast seeks select the next keyframe, not the accurate frame above.
        # Decode sequentially here too, without depending on demuxer seeking.
        set(keyframe_reference "${TEST_WORKDIR}/${variant}-${second}-keyframe-reference.rgb")
        file(REMOVE "${keyframe_reference}")
        run_checked("Extracting ${ENCODER}/${variant} keyframe reference at ${second}s"
            "${FFMPEG}" -nostdin -hide_banner -loglevel error -y
            -threads 1 -i "${video}" -an -filter_threads 1
            -vf "setpts=PTS-STARTPTS,select=gte(t\\,${second})*key,scale=w=96:h=64,format=pix_fmts=rgb24"
            -fps_mode passthrough -frames:v 1 -threads 1 -f rawvideo "${keyframe_reference}"
        )
        file(READ "${keyframe_reference}" keyframe_rgb HEX)
        string(LENGTH "${keyframe_rgb}" keyframe_length)
        if(keyframe_length EQUAL 0)
            # No upcoming keyframe near EOF: generateThumbnail falls back to
            # the first frame instead of returning an accurate non-key frame.
            set(keyframe_hash "${first_frame_hash}")
        elseif(keyframe_length EQUAL 36864)
            file(SHA256 "${keyframe_reference}" keyframe_hash)
        else()
            message(FATAL_ERROR "Expected zero or one 96x64 RGB keyframe reference at ${second}s")
        endif()
        if(second GREATER 0 AND keyframe_hash STREQUAL reference_hash)
            message(FATAL_ERROR "Fixture must distinguish fast and accurate seeks at ${second}s")
        endif()

        # Both fixtures span 20 seconds, so each second is five percent.
        math(EXPR percentage "${second} * 5")
        set(time_thumbnail_hash "")
        set(percentage_thumbnail_hash "")
        foreach(seek_mode time percentage)
            if(seek_mode STREQUAL "time")
                set(seek_argument "00:00:${second}")
                set(expected_hash "${reference_hash}")
            else()
                set(seek_argument "${percentage}")
                set(expected_hash "${keyframe_hash}")
            endif()
            set(thumbnail "${TEST_WORKDIR}/${variant}-${second}-${seek_mode}-thumbnail.rgb")
            file(REMOVE "${thumbnail}")
            set(case "${variant}/${second}s (${seek_mode} -t ${seek_argument})")

            # Write RGB to a file since absolute-time seeking also prints a
            # status line to stdout. A bare number means percentage to the CLI.
            execute_process(
                COMMAND "${THUMBNAILER}" -i "${video}" -o "${thumbnail}"
                    -c rgb -s 0 -t "${seek_argument}"
                RESULT_VARIABLE result
                OUTPUT_VARIABLE stdout
                ERROR_VARIABLE stderr
                TIMEOUT 10
            )
            if(NOT "${result}" STREQUAL "0" OR NOT EXISTS "${thumbnail}")
                list(APPEND failures "${case}: thumbnailing failed (${result}): ${stderr}")
            else()
                file(SHA256 "${thumbnail}" thumbnail_hash)
                set(${seek_mode}_thumbnail_hash "${thumbnail_hash}")
                if(NOT thumbnail_hash STREQUAL expected_hash)
                    list(APPEND failures "${case}: RGB differs from FFmpeg reference (stderr: ${stderr})")
                endif()
            endif()
        endforeach()
        if(second GREATER 0 AND time_thumbnail_hash AND percentage_thumbnail_hash
                AND time_thumbnail_hash STREQUAL percentage_thumbnail_hash)
            list(APPEND failures "${variant}/${second}s: percentage and accurate time thumbnails must differ inside the GOP")
        endif()
    endforeach()
endforeach()

if(failures)
    string(REPLACE ";" "\n  " details "${failures}")
    message(FATAL_ERROR "${ENCODER} accurate-time/fast-percentage seeking regressions:\n  ${details}")
endif()
message(STATUS "${ENCODER}: all 24 accurate-time/fast-percentage comparisons matched FFmpeg; modes differed at all 10 nonzero targets")
