#pragma once

// P2P Core 库导出宏定义
// 用于控制符号在DLL/SO中的导出和导入

#if defined(_WIN32) || defined(_WIN64)
    // Windows 平台
    #if defined(P2P_CORE_BUILD)
        // 构建库时导出符号
        #define P2P_CORE_API __declspec(dllexport)
        #define P2P_CORE_EXTERN extern __declspec(dllexport)
        #define P2P_API __declspec(dllexport)
    #else
        // 使用库时导入符号
        #define P2P_CORE_API __declspec(dllimport)
        #define P2P_CORE_EXTERN extern __declspec(dllimport)
        #define P2P_API __declspec(dllimport)
    #endif
#else
    // Linux/Unix 平台
    #if defined(P2P_CORE_BUILD)
        // 构建库时导出符号
        #define P2P_CORE_API __attribute__((visibility("default")))
        #define P2P_CORE_EXTERN extern __attribute__((visibility("default")))
        #define P2P_API __attribute__((visibility("default")))
    #else
        // 使用库时导入符号
        #define P2P_CORE_API
        #define P2P_CORE_EXTERN extern
        #define P2P_API
    #endif
#endif

// 内联函数不需要导出
#define P2P_CORE_INLINE inline

// 内部使用的符号（不导出）
#define P2P_CORE_INTERNAL __attribute__((visibility("hidden")))

// 版本信息宏
#define P2P_CORE_VERSION_MAJOR 1
#define P2P_CORE_VERSION_MINOR 0
#define P2P_CORE_VERSION_PATCH 0

#define P2P_CORE_VERSION_STRING "1.0.0"


