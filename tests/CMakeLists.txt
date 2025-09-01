# 测试CMake配置

# 查找Google Test (通过FetchContent下载)
# 不需要find_package，因为GTest已经通过FetchContent可用

# 单元测试
file(GLOB_RECURSE UNIT_TEST_SOURCES "unit/*.cpp")
if(UNIT_TEST_SOURCES)
    add_executable(unit_tests ${UNIT_TEST_SOURCES})
    target_link_libraries(unit_tests p2p_core GTest::gtest GTest::gtest_main)
    add_test(NAME UnitTests COMMAND unit_tests)
endif()

# 性能测试
file(GLOB_RECURSE PERF_TEST_SOURCES "performance/*.cpp")
if(PERF_TEST_SOURCES)
    add_executable(performance_tests ${PERF_TEST_SOURCES})
    target_link_libraries(performance_tests p2p_core GTest::gtest GTest::gtest_main)
    add_test(NAME PerformanceTests COMMAND performance_tests)
endif()

# 集成测试
file(GLOB_RECURSE INTEGRATION_TEST_SOURCES "integration/*.cpp")
if(INTEGRATION_TEST_SOURCES)
    add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
    target_link_libraries(integration_tests p2p_core GTest::gtest GTest::gtest_main)
    add_test(NAME IntegrationTests COMMAND integration_tests)
endif()
