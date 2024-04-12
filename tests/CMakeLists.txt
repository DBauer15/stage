enable_testing()

add_executable(
    test_stage
    test_common.cpp
    test_buffer.cpp
    test_mesh.cpp
)
target_link_libraries(
    test_stage
    GTest::gtest_main
    stage
)

include(GoogleTest)
gtest_discover_tests(test_stage)