# 示例代码CMake配置

# 基础使用示例
file(GLOB BASIC_CPP_EXAMPLES "basic_usage/*.cpp")
file(GLOB BASIC_C_EXAMPLES "basic_usage/*.c")

# C++示例
foreach(EXAMPLE ${BASIC_CPP_EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE})
    target_link_libraries(${EXAMPLE_NAME} p2p_core)
    target_include_directories(${EXAMPLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_directories(${EXAMPLE_NAME} PRIVATE ${CMAKE_BINARY_DIR}/src/Debug)
endforeach()

# C示例
foreach(EXAMPLE ${BASIC_C_EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE})
    if(MSVC)
        set_target_properties(${EXAMPLE_NAME} PROPERTIES COMPILE_FLAGS "/TC")
    else()
        set_target_properties(${EXAMPLE_NAME} PROPERTIES COMPILE_FLAGS "-x c")
    endif()
    target_link_libraries(${EXAMPLE_NAME} p2p_core)
    target_include_directories(${EXAMPLE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endforeach()

# 打印调试信息
message(STATUS "BASIC_CPP_EXAMPLES: ${BASIC_CPP_EXAMPLES}")
message(STATUS "BASIC_C_EXAMPLES: ${BASIC_C_EXAMPLES}")

# 高级功能示例
file(GLOB_RECURSE ADVANCED_EXAMPLES "advanced_features/*.cpp")
foreach(EXAMPLE ${ADVANCED_EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE})
    target_link_libraries(${EXAMPLE_NAME} p2p_core)
endforeach()

# 集成示例
file(GLOB_RECURSE INTEGRATION_EXAMPLES "integration/*.cpp")
foreach(EXAMPLE ${INTEGRATION_EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE})
    target_link_libraries(${EXAMPLE_NAME} p2p_core)
endforeach()

# 性能基准测试
file(GLOB_RECURSE BENCHMARK_EXAMPLES "benchmarks/*.cpp")
foreach(EXAMPLE ${BENCHMARK_EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE})
    target_link_libraries(${EXAMPLE_NAME} p2p_core)
endforeach()
