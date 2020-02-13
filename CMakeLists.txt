cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

project(libcqasm
    VERSION 0.0.1
    LANGUAGES CXX
)

option(
    BUILD_SHARED_LIBS
    "whether the library should be built as a shared object or as a static library"
    OFF
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
    cqasm
    "${CMAKE_CURRENT_SOURCE_DIR}/src/cqasm.cpp"
)

target_include_directories(
    cqasm
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set(BISON_VERSION_REQUIRED 3.0)
set(FLEX_VERSION_REQUIRED 2.6.0)
include(cmake/flex-bison.cmake)
