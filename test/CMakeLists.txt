
add_executable(example example.cpp)
target_link_libraries(example gtest_main cqasm)
add_test(
    NAME example_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND example
)

add_executable(compat compat.cpp)
target_link_libraries(compat gtest_main cqasm)
add_test(
    NAME compat_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND compat
)
