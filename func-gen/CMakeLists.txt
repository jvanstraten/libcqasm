cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

# Generator for functions usable within cQASM.
add_executable(
    func-gen
    "${CMAKE_CURRENT_SOURCE_DIR}/func-gen.cpp"
)

target_include_directories(
    func-gen
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
)

function(generate_funcs HDR SRC)

    # Get the directory for the header file and make sure it exists.
    get_filename_component(HDR_DIR "${HDR}" PATH)
    file(MAKE_DIRECTORY "${HDR_DIR}")

    # Get the directory for the source file and make sure it exists.
    get_filename_component(SRC_DIR "${SRC}" PATH)
    file(MAKE_DIRECTORY "${SRC_DIR}")

    # Add a command to do the generation. Note the $<TARGET_FILE:func-gen>
    # dependency; this makes cmake correctly recompile the generator and
    # regenerate the files with it if its source files change.
    add_custom_command(
        COMMAND func-gen "${HDR}" "${SRC}"
        OUTPUT "${HDR}" "${SRC}"
        DEPENDS func-gen
    )

endfunction()
