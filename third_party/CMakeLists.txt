# 第三方依赖CMake配置 - 简化版本

# OpenSSL配置 - 暂时禁用
option(ENABLE_OPENSSL "Enable OpenSSL support" OFF)

if(ENABLE_OPENSSL)
    # 尝试查找系统OpenSSL
    find_package(OpenSSL REQUIRED)
    message(STATUS "OpenSSL enabled")
else()
    message(STATUS "OpenSSL disabled - some features may not work")
    # 创建空的OpenSSL变量以避免链接错误
    set(OPENSSL_INCLUDE_DIR "")
    set(OPENSSL_CRYPTO_LIBRARY "")
    set(OPENSSL_SSL_LIBRARY "")
endif()

# Google Test (通过主CMakeLists.txt的FetchContent处理)
# 这里不需要find_package(GTest REQUIRED)

# MSQUIC (可选)
option(ENABLE_MSQUIC "Enable MSQUIC support" OFF)
if(ENABLE_MSQUIC)
    find_package(MSQUIC REQUIRED)
endif()

# 其他依赖库可以在这里添加
