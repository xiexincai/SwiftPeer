@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo SwiftPeer P2P Core - 仅构建Linux版本
echo ========================================
echo 目标: 仅构建Linux版本 (.so文件)
echo.

echo [信息] 开始构建Linux版本...
echo.

REM 检查WSL2环境
where wsl >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到WSL2，请先安装WSL2
    echo 运行: wsl --install
    pause
    exit /b 1
)

REM 检查Ubuntu发行版
wsl --list --quiet | findstr "Ubuntu" >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未找到Ubuntu发行版
    echo 请先运行: fix_linux_build.bat
    pause
    exit /b 1
)

echo ========================================
echo 步骤1: 准备构建环境
echo ========================================
echo.

echo [信息] 准备构建环境...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo '准备构建环境...' && if ! command -v cmake >/dev/null 2>&1; then echo '安装CMake...'; sudo apt-get update; sudo apt-get install -y cmake build-essential; fi && if [ -d 'build_linux' ]; then echo '清理旧的构建目录...'; rm -rf build_linux; fi && mkdir -p build_linux && cd build_linux && echo '构建环境准备完成'"

echo ========================================
echo 步骤2: 配置项目
echo ========================================
echo.

echo [信息] 配置Linux项目...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer/build_linux && echo '配置Linux项目...' && cmake .. -DCMAKE_BUILD_TYPE=Release && if [ $? -eq 0 ]; then echo '项目配置成功'; else echo '项目配置失败'; exit 1; fi"

if %errorlevel% neq 0 (
    echo [错误] 项目配置失败
    echo 请检查CMakeLists.txt和源代码
    pause
    exit /b 1
)

echo ========================================
echo 步骤3: 构建项目
echo ========================================
echo.

echo [信息] 构建Linux项目...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer/build_linux && echo '构建Linux项目...' && cmake --build . -j$(nproc) && if [ $? -eq 0 ]; then echo '项目构建成功'; ls -la; else echo '项目构建失败'; exit 1; fi"

if %errorlevel% neq 0 (
    echo [错误] 项目构建失败
    echo 请检查源代码和依赖
    pause
    exit /b 1
)

echo ========================================
echo 步骤4: 检查构建结果
echo ========================================
echo.

echo [信息] 检查构建结果...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer/build_linux && echo '检查构建结果...' && if [ -f 'src/libp2p_core.so' ]; then echo '找到共享库: src/libp2p_core.so'; ls -la src/libp2p_core.so; else echo '未找到共享库文件'; echo '查找所有.so文件:'; find . -name '*.so' -type f; fi && echo '所有生成的文件:'; find . -type f -name '*.so' -o -name '*.a' -o -name '*.o'"

echo ========================================
echo 构建完成
echo ========================================
echo.

echo [成功] Linux版本构建完成！
echo.
echo 构建结果:
echo   - 构建目录: build_linux/
echo   - 共享库: build_linux/src/libp2p_core.so
echo   - 头文件: include/p2p/
echo.

echo 使用方法:
echo   1. 在Linux项目中使用: libp2p_core.so
echo   2. 包含头文件目录: include/p2p/
echo   3. 链接时使用: -lp2p_core
echo.

pause
goto :eof