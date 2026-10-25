file(MAKE_DIRECTORY "${TEST_WORKDIR}")

function(check_missing_file filename escaped_filename)
    set(input "${TEST_WORKDIR}/missing-input/${filename}")
    set(output "${TEST_WORKDIR}/thumbnail.rgb")
    if(EXISTS "${input}")
        message(FATAL_ERROR "Test input must not exist: ${input}")
    endif()

    execute_process(
        COMMAND "${THUMBNAILER}" -i "${input}" -o "${output}" -c rgb
        RESULT_VARIABLE result
        OUTPUT_VARIABLE stdout
        ERROR_VARIABLE stderr
        TIMEOUT 10
    )
    file(REMOVE "${output}")

    if("${result}" STREQUAL "0")
        message(FATAL_ERROR "Thumbnailing a missing input unexpectedly succeeded")
    endif()
    if(NOT "${stdout}" STREQUAL "")
        message(FATAL_ERROR "Missing-file diagnostics must not be written to stdout")
    endif()

    set(expected "Error: Could not open input file: ${TEST_WORKDIR}/missing-input/${escaped_filename}\n")
    if(NOT "${stderr}" STREQUAL "${expected}")
        # Do not render potentially unsafe bytes if escaping regresses.
        message(FATAL_ERROR "Missing-file diagnostic did not match the expected escaped filename")
    endif()
endfunction()

string(ASCII 27 esc)
string(ASCII 7 bell)
string(ASCII 127 del)
check_missing_file(
    "foo_${esc}]0;TITLE${bell}\n\t\r${del}.mp4"
    "foo_\\x1B]0;TITLE\\x07\\x0A\\x09\\x0D\\x7F.mp4"
)
check_missing_file("Кругом_шумел_日本語_🎬.mp4" "Кругом_шумел_日本語_🎬.mp4")
