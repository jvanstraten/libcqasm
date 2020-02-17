
add_executable(example example.cpp)
target_link_libraries(example gtest_main cqasm)
add_test(NAME example_test COMMAND example)
