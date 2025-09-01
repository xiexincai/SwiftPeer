# SwiftPeer P2P Core - 跨平台构建指南

## 概述

本指南介绍如何在Windows平台上构建SwiftPeer P2P Core项目，同时生成Windows DLL和Linux SO文件。本项目提供了完整的跨平台构建解决方案，支持WSL2和Docker两种跨平台编译环境。

## 🚀 快速开始

### 第一次使用

1. **设置WSL2环境**：
   ```cmd
   setup_wsl2.bat
   ```

2. **快速跨平台构建**：
   ```cmd
   build_cross_quick.bat
   ```

### 日常使用

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

## 📋 构建脚本说明

### 主要构建脚本

#### 1. `build_cross_quick.bat` - 快速跨平台构建
- **用途**: 日常开发使用，自动检测环境并构建
- **功能**: 
  - 自动检测WSL2或Docker环境
  - 构建Windows DLL和Linux SO文件
  - 简单的错误处理和状态显示
- **使用场景**: 快速验证代码，日常开发构建

#### 2. `build_universal.bat` - 通用跨平台构建
- **用途**: 高级用户和CI/CD环境
- **功能**:
  - 支持多种构建配置
  - 支持命令行参数
  - 详细的构建日志
  - 灵活的构建选项
- **使用场景**: 生产构建，自动化脚本，高级配置

#### 3. `build_linux_only.bat` - 仅构建Linux版本
- **用途**: 专门用于Linux SO文件构建
- **功能**:
  - 仅构建Linux版本
  - 简化的构建流程
  - 快速Linux构建
- **使用场景**: 调试Linux构建问题，仅需要Linux版本

### 环境设置脚本

#### 4. `setup_wsl2.bat` - WSL2环境设置
- **用途**: 自动安装和配置WSL2环境
- **功能**:
  - 自动安装WSL2
  - 安装Ubuntu 22.04 LTS
  - 安装必要的构建工具
  - 配置跨平台编译环境
- **使用场景**: 首次设置，环境修复

#### 5. `setup_docker.bat` - Docker环境设置
- **用途**: 配置Docker环境用于跨平台编译
- **功能**:
  - 检查Docker安装状态
  - 配置Docker构建环境
  - 测试Docker跨平台编译
- **使用场景**: 使用Docker进行跨平台构建

### 诊断和修复脚本

#### 6. `diagnose_linux_build.bat` - Linux构建诊断
- **用途**: 诊断Linux构建问题
- **功能**:
  - 检查WSL2环境状态
  - 检查构建工具安装
  - 测试项目配置
  - 提供详细的诊断报告
- **使用场景**: 构建失败时的问题诊断

#### 7. `fix_linux_build.bat` - Linux构建修复
- **用途**: 自动修复Linux构建问题
- **功能**:
  - 自动修复常见问题
  - 安装缺失的构建工具
  - 测试构建环境
  - 提供修复建议
- **使用场景**: 自动修复构建环境问题

## 🔧 构建参数

### build_universal.bat 参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `--debug` | 构建调试版本 | Release |
| `--release` | 构建发布版本 | ✓ |
| `--clean` | 清理构建目录 | false |
| `--verbose` | 详细输出 | false |
| `--jobs N` | 并行作业数 | 4 |
| `--help` | 显示帮助信息 | - |

### 使用示例

```cmd
# 构建发布版本并清理
build_universal.bat --release --clean

# 构建调试版本，详细输出，8个并行作业
build_universal.bat --debug --verbose --jobs 8

# 显示帮助信息
build_universal.bat --help
```

## 📊 构建结果

### Windows版本
- **DLL文件**: `build_cross_windows/src/p2p_core.dll`
- **导入库**: `build_cross_windows/src/p2p_core.lib`
- **头文件**: `include/p2p/`

### Linux版本
- **共享库**: `build_cross_linux/src/libp2p_core.so`
- **头文件**: `include/p2p/`

## 🛠️ 环境要求

### Windows环境
- **操作系统**: Windows 10/11
- **编译器**: Visual Studio 2022 (或更新版本)
- **构建工具**: CMake 3.16+
- **跨平台环境**: WSL2 或 Docker

### WSL2环境
- **发行版**: Ubuntu 22.04 LTS
- **构建工具**: build-essential, cmake, git
- **权限**: 管理员权限（用于安装）

### Docker环境
- **Docker**: Docker Desktop for Windows
- **镜像**: Ubuntu 20.04+
- **权限**: Docker运行权限

## 🔍 故障排除

### 常见问题及解决方案

#### 1. WSL2未安装
**症状**: 脚本提示"未找到WSL2"
**解决方案**: 
```cmd
# 运行WSL2安装脚本
setup_wsl2.bat

# 或手动安装
wsl --install
```

#### 2. Ubuntu未安装
**症状**: 脚本提示"未找到Ubuntu发行版"
**解决方案**:
```cmd
# 运行环境设置脚本
setup_wsl2.bat

# 或手动安装
wsl --install -d Ubuntu-22.04
```

#### 3. 构建工具缺失
**症状**: Linux构建失败，提示找不到cmake/gcc
**解决方案**:
```cmd
# 运行修复脚本
fix_linux_build.bat

# 或手动安装
wsl
sudo apt-get update
sudo apt-get install -y build-essential cmake git
```

#### 4. 项目配置失败
**症状**: CMake配置失败
**解决方案**:
1. 检查CMakeLists.txt是否存在
2. 检查源代码和头文件目录
3. 运行诊断脚本：
   ```cmd
   diagnose_linux_build.bat
   ```

#### 5. 权限问题
**症状**: WSL2无法访问Windows文件
**解决方案**:
1. 确保WSL2有访问Windows文件的权限
2. 检查文件权限设置
3. 以管理员身份运行脚本

#### 6. 编码问题
**症状**: 构建时出现编码错误
**解决方案**:
- 脚本已自动处理编码问题
- 使用 `chcp 65001` 设置UTF-8编码
- 确保源代码文件使用UTF-8编码

## 📝 手动构建

### Windows版本
```cmd
mkdir build_windows
cd build_windows
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/utf-8"
cmake --build . --config Release
```

### Linux版本
```cmd
wsl
cd /mnt/d/Develop/Project/SwiftPeer
mkdir -p build_linux
cd build_linux
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

## 🔄 工作流程

### 开发工作流程

1. **代码修改**: 在Windows上编辑源代码
2. **快速构建**: 运行 `build_cross_quick.bat`
3. **测试验证**: 在Windows和Linux上测试
4. **问题诊断**: 如有问题运行 `diagnose_linux_build.bat`
5. **环境修复**: 如有问题运行 `fix_linux_build.bat`

### CI/CD集成

```yaml
# GitHub Actions 示例
name: Cross Platform Build
on: [push, pull_request]
jobs:
  build:
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v3
    - name: Setup WSL2
      run: setup_wsl2.bat
    - name: Build Cross Platform
      run: build_universal.bat --release --clean
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: p2p-core-libs
        path: |
          build_cross_windows/src/p2p_core.dll
          build_cross_linux/src/libp2p_core.so
```

## 📚 最佳实践

### 1. 环境管理
- 定期更新WSL2和Ubuntu
- 保持构建工具版本一致
- 使用版本控制管理构建配置

### 2. 构建优化
- 使用 `--jobs` 参数优化并行构建
- 定期清理构建目录
- 使用 `--verbose` 参数调试构建问题

### 3. 错误处理
- 遇到问题时先运行诊断脚本
- 查看详细的错误日志
- 使用修复脚本自动解决问题

### 4. 版本控制
- 将构建脚本纳入版本控制
- 记录构建配置变更
- 维护构建环境的一致性

## 🆘 技术支持

如果遇到问题，请按以下步骤操作：

1. **运行诊断脚本**：
   ```cmd
   diagnose_linux_build.bat
   ```

2. **尝试自动修复**：
   ```cmd
   fix_linux_build.bat
   ```

3. **查看构建日志**：检查详细的错误信息

4. **检查环境要求**：确保满足所有环境要求

5. **手动验证**：使用手动构建命令验证环境

## 📈 更新日志

- **v1.0** - 初始版本，支持基本的跨平台构建
- **v1.1** - 添加了诊断和修复脚本
- **v1.2** - 优化了脚本语法，修复了编码问题
- **v1.3** - 合并了所有修复版本，简化了脚本结构
- **v1.4** - 完善了文档和故障排除指南

---

**注意**: 本指南基于Windows 10/11和WSL2环境编写。如果您使用其他环境，可能需要调整相关配置。