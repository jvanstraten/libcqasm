cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(libcqasm CXX)

# Library type option. Default is a static library.
option(
    BUILD_SHARED_LIBS
    "whether the library should be built as a shared object or as a static library"
    OFF
)

option(
    BUILD_TESTS
    "whether the tests should be built and added to `make test`"
    OFF
)

# Require C++11 compiler support.
if(CMAKE_VERSION VERSION_LESS "3.1")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS ON)
endif()

# Require flex/bison; if not installed, this will try to build from source.
set(FLEX_VERSION_REQUIRED 2.6.1)
set(BISON_VERSION_REQUIRED 3.0)
include(cmake/flex-bison.cmake)

# Generate the lexer.
flex_target(
    cqasm-lexer
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cqasm-lexer.l"
    "${CMAKE_CURRENT_BINARY_DIR}/cqasm-lexer.cpp"
    DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/cqasm-lexer.hpp"
)

# Generate the parser.
bison_target(
    cqasm-parser
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cqasm-parser.y"
    "${CMAKE_CURRENT_BINARY_DIR}/cqasm-parser.cpp"
)

# Dependency between lexer and parser.
add_flex_bison_dependency(
    cqasm-lexer
    cqasm-parser
)

# Main cQASM library.
add_library(
    cqasm
    ${BISON_cqasm-parser_OUTPUTS}
    ${FLEX_cqasm-lexer_OUTPUTS}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cqasm.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cqasm-ast.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cqasm-analyzer.cpp"
)

# The src directory and binary directory (for generated header files) are
# private. The include directory is public. That is, the files in include
# must not try to include generated files and files from the src directory,
# but the other direction is fine.
target_include_directories(
    cqasm
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Add the test directory.
if(BUILD_TESTS)
    include(cmake/googletest.cmake)
    enable_testing()
    add_subdirectory(test)
endif()
