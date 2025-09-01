# 源文件CMake配置 - 优化版本，支持DLL/SO导出

# 收集所有源文件，暂时排除有问题的文件
file(GLOB_RECURSE ALL_SOURCES "*.cpp")
# 暂时排除有问题的文件
list(REMOVE_ITEM ALL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/enhanced_dht.cpp")
list(REMOVE_ITEM ALL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/enhanced_nat_traversal.cpp")

# 创建共享库
add_library(p2p_core SHARED ${ALL_SOURCES})

# 设置库属性
set_target_properties(p2p_core PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    OUTPUT_NAME "p2p_core"
    DEBUG_POSTFIX "_d"
    RELEASE_POSTFIX ""
)

# 跨平台库命名和导出配置
if(WIN32)
    # Windows平台配置
    set_target_properties(p2p_core PROPERTIES
        PREFIX ""
        IMPORT_PREFIX ""
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
    
    # 设置Windows特定的编译定义
    target_compile_definitions(p2p_core PRIVATE 
        P2P_CORE_BUILD
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
else()
    # Linux/Unix平台配置
    set_target_properties(p2p_core PROPERTIES
        PREFIX "lib"
        IMPORT_PREFIX "lib"
    )
    
    # 设置Linux特定的编译定义
    target_compile_definitions(p2p_core PRIVATE P2P_CORE_BUILD)
    
    # 设置符号可见性
    target_compile_options(p2p_core PRIVATE -fvisibility=hidden)
    
    # 设置rpath
    set_target_properties(p2p_core PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# 查找必要的包
find_package(Threads REQUIRED)

# 跨平台链接库配置
if(WIN32)
    target_link_libraries(p2p_core 
        Threads::Threads 
        ws2_32
        iphlpapi
        crypt32
    )
else()
    target_link_libraries(p2p_core 
        Threads::Threads
        dl
        rt
    )
    
    # 检查并链接必要的系统库
    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_libraries(p2p_core m)
    endif()
endif()

# 设置包含目录
target_include_directories(p2p_core PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

# 安装配置
install(TARGETS p2p_core
    EXPORT p2p_coreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# 安装头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 导出目标
install(EXPORT p2p_coreTargets
    FILE p2p_coreTargets.cmake
    NAMESPACE p2p_core::
    DESTINATION lib/cmake/p2p_core
)

# 创建配置文件
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/p2p_coreConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/p2p_coreConfig.cmake"
    INSTALL_DESTINATION lib/cmake/p2p_core
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/p2p_coreConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

# 安装配置文件
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/p2p_coreConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/p2p_coreConfigVersion.cmake"
    DESTINATION lib/cmake/p2p_core
)

# 打印构建信息
message(STATUS "构建目标: p2p_core")
message(STATUS "库类型: SHARED")
if(WIN32)
    message(STATUS "Windows库: p2p_core.dll")
else()
    message(STATUS "Linux库: libp2p_core.so")
endif()
