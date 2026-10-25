cmake_minimum_required(VERSION 3.12)

# Issue #236: PQ/HLG need CPU tone mapping; BT.2020 primaries alone do not.
# Generate tiny lossless fixtures locally, without mastering/light-level side
# data, so tonemap must use FFmpeg's automatic peak fallback (no fixed peak).
function(run_checked description)
    execute_process(
        COMMAND ${ARGN}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
        TIMEOUT 15
    )
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${description} failed (${result}):\n${stdout}\n${stderr}")
    endif()
    set(command_stderr "${stderr}" PARENT_SCOPE)
endfunction()

execute_process(
    COMMAND "${FFMPEG}" -hide_banner -filters
    RESULT_VARIABLE result
    OUTPUT_VARIABLE filters
    ERROR_VARIABLE stderr
    TIMEOUT 10
)
if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "Could not list FFmpeg filters (${result}): ${stderr}")
endif()
foreach(filter zscale tonemap)
    if(NOT filters MATCHES "[ \t]${filter}[ \t]")
        message("SKIP: HDR prerequisite filter ${filter} is unavailable")
        return()
    endif()
endforeach()
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
if(NOT encoders MATCHES "[ \t]ffv1[ \t]")
    message("SKIP: HDR prerequisite encoder ffv1 is unavailable")
    return()
endif()

file(MAKE_DIRECTORY "${TEST_WORKDIR}")
set(hdr_chain "zscale=transfer=linear:npl=100,format=pix_fmts=gbrpf32le,zscale=primaries=bt709,tonemap=tonemap=mobius:desat=0,zscale=transfer=iec61966-2-1:primaries=bt709:matrix=gbr:range=full:dither=error_diffusion,format=pix_fmts=gbrp")

function(rgb_hash path width height hash_variable)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Missing RGB output: ${path}")
    endif()
    # Packed RGB must omit AVFrame row padding, notably at odd output widths.
    file(READ "${path}" rgb HEX)
    string(LENGTH "${rgb}" length)
    math(EXPR expected_length "${width} * ${height} * 3 * 2")
    if(NOT length EQUAL expected_length)
        message(FATAL_ERROR "${path}: expected one packed ${width}x${height} RGB frame (${expected_length} hex digits), got ${length}")
    endif()
    file(SHA256 "${path}" hash)
    set(${hash_variable} "${hash}" PARENT_SCOPE)
endfunction()

set(failures "")
foreach(transfer smpte2084 arib-std-b67 bt2020-10)
    set(video "${TEST_WORKDIR}/${transfer}.mkv")
    # Legal-range 10-bit gradients cover shadows, highlights, and chroma. Only
    # the transfer differs: all three fixtures are limited-range BT.2020 NCL.
    run_checked("Generating ${transfer} FFV1 fixture"
        "${FFMPEG}" -nostdin -hide_banner -loglevel error -y
        -f lavfi -i "nullsrc=size=96x64:rate=1,format=pix_fmts=yuv420p10le,geq=lum='64+876*X/(W-1)':cb='384+256*Y/(H-1)':cr='640-256*X/(W-1)'"
        -vf "setparams=range=limited:color_primaries=bt2020:color_trc=${transfer}:colorspace=bt2020nc"
        -frames:v 1 -an -c:v ffv1 -threads 1 -pix_fmt yuv420p10le
        -color_range tv -colorspace bt2020nc -color_primaries bt2020
        -color_trc "${transfer}" "${video}"
    )
    # Inspect decoded frame metadata rather than assuming Matroska preserved
    # the encoder options. No ffprobe dependency is required.
    run_checked("Checking ${transfer} fixture metadata"
        "${FFMPEG}" -nostdin -hide_banner -loglevel info
        -threads 1 -i "${video}" -an -filter_threads 1 -vf showinfo
        -frames:v 1 -f null -
    )
    foreach(tag "fmt:yuv420p10le" "color_range:tv" "color_space:bt2020nc"
            "color_primaries:bt2020" "color_trc:${transfer}")
        if(NOT command_stderr MATCHES "${tag}([ \t\r\n]|$)")
            message(FATAL_ERROR "${transfer}: decoded fixture is missing ${tag}:\n${command_stderr}")
        endif()
    endforeach()
    if(command_stderr MATCHES "[Mm]astering display|[Cc]ontent light")
        message(FATAL_ERROR "${transfer}: fixture must exercise missing HDR peak metadata")
    endif()

    foreach(size 0 31)
        if(size EQUAL 0)
            set(width 96)
            set(height 64)
        else()
            set(width 31)
            set(height 21) # scale's nearest-integer aspect-ratio rounding
        endif()
        set(case "${transfer}/size-${size}")
        set(plain "${TEST_WORKDIR}/${transfer}-${size}-plain.rgb")
        set(mapped "${TEST_WORKDIR}/${transfer}-${size}-mapped.rgb")
        set(thumbnail "${TEST_WORKDIR}/${transfer}-${size}-thumbnail.rgb")
        set(scale_chain "scale=w=${width}:h=-1,format=pix_fmts=rgb24")
        foreach(mode plain mapped)
            if(mode STREQUAL "mapped")
                set(chain "${hdr_chain},${scale_chain}")
            else()
                set(chain "${scale_chain}")
            endif()
            set(reference "${${mode}}")
            file(REMOVE "${reference}")
            run_checked("Extracting ${case} ${mode} reference"
                "${FFMPEG}" -nostdin -hide_banner -loglevel error -y
                -threads 1 -i "${video}" -an -filter_threads 1 -vf "${chain}"
                -frames:v 1 -threads 1 -f rawvideo "${reference}"
            )
            rgb_hash("${reference}" "${width}" "${height}" ${mode}_hash)
        endforeach()
        if(mapped_hash STREQUAL plain_hash)
            message(FATAL_ERROR "${case}: fixture must distinguish tone-mapped and untonemapped RGB")
        endif()
        if(transfer STREQUAL "bt2020-10")
            set(expected_hash "${plain_hash}")
        else()
            set(expected_hash "${mapped_hash}")
        endif()

        file(REMOVE "${thumbnail}")
        execute_process(
            COMMAND "${THUMBNAILER}" -i "${video}" -o "${thumbnail}"
                -c rgb -s "${size}" -t 0
            RESULT_VARIABLE result
            OUTPUT_VARIABLE stdout
            ERROR_VARIABLE stderr
            TIMEOUT 10
        )
        if(NOT "${result}" STREQUAL "0" OR NOT EXISTS "${thumbnail}")
            list(APPEND failures "${case}: thumbnailing failed (${result}): ${stdout} ${stderr}")
        else()
            rgb_hash("${thumbnail}" "${width}" "${height}" thumbnail_hash)
            if(NOT thumbnail_hash STREQUAL expected_hash)
                list(APPEND failures "${case}: RGB differs from FFmpeg reference (stderr: ${stderr})")
            endif()
        endif()
    endforeach()
endforeach()

if(failures)
    string(REPLACE ";" "\n  " details "${failures}")
    message(FATAL_ERROR "HDR/SDR tone-mapping regressions:\n  ${details}")
endif()
message(STATUS "PQ/HLG tone mapping and untouched BT.2020 SDR: all 6 packed-RGB comparisons matched FFmpeg")
