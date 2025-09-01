# SwiftPeer - 高性能P2P网络库

[![CMake](https://github.com/xiexincai/SwiftPeer/actions/workflows/cmake.yml/badge.svg)](https://github.com/xiexincai/SwiftPeer/actions/workflows/cmake.yml)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20Android-blue.svg)](https://github.com/xiexincai/SwiftPeer)

SwiftPeer是一个轻量、灵活的跨平台P2P网络库，提供高性能、易集成的P2P通信解决方案。

## ✨ 核心特性

- **🚀 高性能**: 支持多种传输协议（TCP、QUIC、TLS），优化网络性能
- **🔒 高安全性**: TLS加密、HMAC认证、ECDSA签名验证
- **📱 轻量级**: 专为移动平台优化，内存占用小，启动快速
- **🔌 易集成**: 提供C和C++两种API接口，支持多种集成方式
- **🌐 跨平台**: 支持Windows、Linux、Android NDK平台
- **📊 可监控**: 内置统计监控，支持性能分析和调试
- **🔄 可扩展**: 模块化架构，支持插件扩展和自定义功能

## 🚀 快速开始

### 系统要求

- **编译器**: Visual Studio 2019+ (Windows) 或 GCC 7+ (Linux)
- **C++标准**: C++17
- **CMake**: 3.16+
- **平台**: Windows 10+, Linux, Android NDK
- **跨平台构建**: WSL2 或 Docker (用于Linux构建)

### 跨平台构建

本项目提供了完整的跨平台构建解决方案，可以在Windows平台上同时生成Windows DLL和Linux SO文件。

#### 第一次使用

1. **设置WSL2环境**：
   ```cmd
   setup_wsl2.bat
   ```

2. **快速跨平台构建**：
   ```cmd
   build_cross_quick.bat
   ```

#### 日常使用

1. **快速跨平台构建**：
   ```cmd
   build_cross_quick.bat
   ```

2. **高级跨平台构建**：
   ```cmd
   build_universal.bat --release --clean
   ```

3. **仅构建Linux版本**：
   ```cmd
   build_linux_only.bat
   ```

#### 故障排除

1. **诊断Linux构建问题**：
   ```cmd
   diagnose_linux_build.bat
   ```

2. **自动修复Linux构建环境**：
   ```cmd
   fix_linux_build.bat
   ```

### 传统构建方式

#### Windows (Visual Studio)

```bash
# 克隆项目
git clone https://github.com/xiexincai/SwiftPeer.git
cd SwiftPeer

# 配置和编译
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# 运行测试 (可选)
cmake --build build --target test
```

#### Linux

```bash
# 克隆项目
git clone https://github.com/xiexincai/SwiftPeer.git
cd SwiftPeer

# 配置和编译
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 运行测试 (可选)
cmake --build build --target test
```

## 📁 项目结构

```
SwiftPeer/
├── include/                 # 头文件
│   └── p2p/               # P2P库头文件
│       ├── core/          # 核心功能
│       ├── transport/     # 传输层
│       ├── discovery/     # 发现与路由
│       ├── data/          # 数据管理
│       ├── security/      # 安全功能
│       └── utils/         # 工具函数
├── src/                    # 源代码
│   ├── core/              # 核心实现
│   ├── transport/         # 传输层实现
│   ├── discovery/         # 发现与路由实现
│   ├── data/              # 数据管理实现
│   ├── security/          # 安全功能实现
│   └── utils/             # 工具函数实现
├── examples/               # 示例代码
│   ├── basic_usage/       # 基本用法示例
│   ├── advanced_features/ # 高级功能示例
│   ├── integration/       # 集成示例
│   └── benchmarks/        # 性能测试
├── tests/                  # 测试代码
│   ├── fixtures/          # 测试数据
│   ├── integration/       # 集成测试
│   ├── mocks/             # 模拟对象
│   ├── performance/       # 性能测试
│   └── unit/              # 单元测试
├── scripts/                # 构建和部署脚本
│   ├── build/             # 构建脚本
│   ├── ci/                # CI/CD脚本
│   ├── deploy/            # 部署脚本
│   └── test/              # 测试脚本
├── tools/                  # 开发工具
│   ├── build/             # 构建工具
│   ├── codegen/           # 代码生成工具
│   ├── deployment/        # 部署工具
│   └── profiling/         # 性能分析工具
├── third_party/            # 第三方依赖
├── config/                 # 配置文件
├── cmake/                  # CMake模块
├── CMakeLists.txt          # CMake配置
├── README.md               # 项目说明
└── CROSS_PLATFORM_BUILD_GUIDE.md  # 跨平台构建指南
```

## 🔧 构建脚本说明

### 主要构建脚本

- **`build_cross_quick.bat`** - 快速跨平台构建（推荐日常使用）
- **`build_universal.bat`** - 通用跨平台构建（支持命令行参数）
- **`build_linux_only.bat`** - 仅构建Linux版本

### 环境设置脚本

- **`setup_wsl2.bat`** - WSL2环境设置
- **`setup_docker.bat`** - Docker环境设置

### 诊断和修复脚本

- **`diagnose_linux_build.bat`** - Linux构建诊断
- **`fix_linux_build.bat`** - Linux构建修复

### 构建参数

`build_universal.bat` 支持以下参数：

- `--debug` - 构建调试版本
- `--release` - 构建发布版本 (默认)
- `--clean` - 清理构建目录
- `--verbose` - 详细输出
- `--jobs N` - 并行作业数 (默认: 4)
- `--help` - 显示帮助信息

## 📊 构建结果

### Windows版本
- **DLL文件**: `build_cross_windows/src/p2p_core.dll` 或 `build_universal_windows/src/p2p_core.dll`
- **导入库**: `build_cross_windows/src/p2p_core.lib` 或 `build_universal_windows/src/p2p_core.lib`
- **头文件**: `include/p2p/`

### Linux版本
- **共享库**: `build_cross_linux/src/libp2p_core.so` 或 `build_universal_linux/src/libp2p_core.so`
- **头文件**: `include/p2p/`

## 基本使用

### C++ API

```cpp
#include <p2p/core/peer_engine.h>
#include <p2p/core/config.h>

using namespace p2p_core;

int main() {
    // 创建配置
    P2PConfig config;
    config.nodeId = "my_node_001";
    config.listenPort = 0;  // 自动选择端口
    config.maxUploadBps = 1024000;  // 1 MB/s
    config.maxDownloadBps = 0;      // 无限制
    
    // 创建P2P引擎
    PeerEngine engine(config);
    
    // 启动引擎
    if (engine.start()) {
        std::cout << "P2P引擎启动成功" << std::endl;
        
        // 添加种子节点
        engine.add_seed_peer({"127.0.0.1", 5000});
        
        // 发布数据
        PieceData piece;
        piece.id.streamId = "test_stream";
        piece.id.pieceIndex = 1;
        piece.data = {0x01, 0x02, 0x03, 0x04, 0x05};
        engine.publish_piece(piece);
        
        // 运行一段时间
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // 显示统计信息
        auto stats = engine.stats();
        std::cout << "上传: " << stats.bytesUploaded << " bytes" << std::endl;
        std::cout << "下载: " << stats.bytesDownloaded << " bytes" << std::endl;
        std::cout << "连接节点: " << stats.connectedPeers << std::endl;
        
        engine.stop();
    }
    
    return 0;
}
```

### C API

```c
#include <p2p/api/c_api.h>
#include <stdio.h>

int main() {
    // 创建配置
    p2p_config_t config = {0};
    config.node_id = "my_node_001";
    config.listen_port = 0;
    config.max_upload_bps = 1024000;
    
    // 创建P2P实例
    p2p_handle_t handle = p2p_create(&config);
    if (!handle) {
        fprintf(stderr, "无法创建P2P实例\n");
        return 1;
    }
    
    // 启动引擎
    if (p2p_start(handle) == 0) {
        printf("P2P引擎启动成功\n");
        
        // 添加种子节点
        p2p_endpoint_t seed = {"127.0.0.1", 5000};
        p2p_add_seed(handle, &seed);
        
        // 运行一段时间
        for (int i = 0; i < 10; i++) {
            p2p_stats_t stats = p2p_stats(handle);
            printf("连接节点: %u\n", stats.connected_peers);
            sleep(1);
        }
        
        p2p_stop(handle);
    }
    
    p2p_destroy(handle);
    return 0;
}
```

## 🔧 配置选项

### P2P配置参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `nodeId` | string | 自动生成 | 节点唯一标识符 |
| `listenPort` | uint16 | 0 | 监听端口（0为自动选择） |
| `maxUploadBps` | uint64 | 1024000 | 最大上传速率 (bytes/s) |
| `maxDownloadBps` | uint64 | 0 | 最大下载速率 (bytes/s) |
| `maxBurstBytes` | uint64 | 65536 | 最大突发字节数 |
| `maxPeers` | uint32 | 20 | 最大连接节点数 |
| `enableTls` | bool | false | 启用TLS加密 |
| `enableDht` | bool | false | 启用DHT网络 |
| `enableQuic` | bool | false | 启用QUIC传输 |

## 📊 性能特性

- **低延迟**: 优化的网络栈，支持快速连接建立
- **高吞吐**: 支持多线程并发处理，最大化网络利用率
- **内存效率**: 智能内存管理，减少内存碎片和泄漏
- **CPU优化**: 高效的算法实现，最小化CPU占用
- **网络适应**: 自适应速率控制，适应不同网络环境

## 🔒 安全特性

- **传输加密**: TLS 1.3支持，保护数据传输安全
- **身份验证**: ECDSA P-256签名，验证数据来源
- **完整性校验**: SHA-256哈希，确保数据完整性
- **访问控制**: 可配置的访问策略和权限管理
- **隐私保护**: 支持匿名模式和隐私保护功能

## 🧪 测试

### 运行测试

```bash
# 构建测试
cmake --build build --target test

# 运行所有测试
ctest --test-dir build

# 运行特定测试
ctest --test-dir build -R "unit_tests"
```

### 测试覆盖

- **单元测试**: 核心功能模块测试
- **集成测试**: 模块间交互测试
- **性能测试**: 吞吐量和延迟测试
- **压力测试**: 高负载和边界条件测试
- **兼容性测试**: 跨平台兼容性验证

## 📚 文档

- [跨平台构建指南](CROSS_PLATFORM_BUILD_GUIDE.md) - 详细的跨平台构建说明
- [示例代码](examples/) - 丰富的使用示例

## 🤝 贡献指南

我们欢迎所有形式的贡献！请查看 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详情。

### 贡献方式

1. **报告Bug**: 在GitHub Issues中报告问题
2. **功能建议**: 提出新功能或改进建议
3. **代码贡献**: 提交Pull Request
4. **文档改进**: 帮助改进文档和示例
5. **测试贡献**: 添加测试用例或改进测试覆盖

### 开发环境设置

```bash
# 克隆项目
git clone https://github.com/xiexincai/SwiftPeer.git
cd SwiftPeer

# 创建开发分支
git checkout -b feature/your-feature-name

# 设置跨平台构建环境
setup_wsl2.bat

# 构建项目
build_cross_quick.bat

# 运行测试
scripts/test/run_tests.bat
```

## 📄 许可证

本项目采用 [MIT许可证](LICENSE) - 查看LICENSE文件了解详情。

## 🙏 致谢

感谢所有为这个项目做出贡献的开发者和用户！

## 📞 联系我们

- **GitHub Issues**: [报告问题](https://github.com/xiexincai/SwiftPeer/issues)
- **GitHub Discussions**: [参与讨论](https://github.com/xiexincai/SwiftPeer/discussions)
- **Email**: 546093470@qq.com

---

**SwiftPeer** - 让P2P通信更简单、更高效、更安全！ 🚀