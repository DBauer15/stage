enable_testing()

add_executable(
    test_stage
    test_buffer.cpp
)
target_link_libraries(
    test_stage
    GTest::gtest_main
    stage
)

include(GoogleTest)
gtest_discover_tests(test_stage)