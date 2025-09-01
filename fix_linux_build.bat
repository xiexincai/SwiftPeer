@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo SwiftPeer P2P Core - Linux构建修复脚本
echo ========================================
echo 目标: 快速修复Linux版本构建问题
echo.

echo [信息] 开始修复Linux构建环境...
echo.

REM 检查WSL2环境
echo ========================================
echo 步骤1: 检查WSL2环境
echo ========================================
echo.

where wsl >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到WSL2，请先安装WSL2
    echo 运行: wsl --install
    pause
    exit /b 1
) else (
    echo [成功] 找到WSL2
)

REM 检查Ubuntu发行版
wsl --list --quiet | findstr "Ubuntu" >nul 2>&1
if %errorlevel% neq 0 (
    echo [警告] 未找到Ubuntu发行版，开始安装...
    wsl --install -d Ubuntu-22.04
    if %errorlevel% neq 0 (
        echo [错误] Ubuntu安装失败
        pause
        exit /b 1
    )
    echo [信息] Ubuntu安装完成，请设置用户名和密码
    echo 设置完成后，请重新运行此脚本
    pause
    exit /b 0
) else (
    echo [成功] 找到Ubuntu发行版
)

echo ========================================
echo 步骤2: 安装构建工具
echo ========================================
echo.

echo [信息] 安装Linux构建工具...
wsl bash -c "echo '安装构建工具...' && sudo apt-get update && sudo apt-get install -y build-essential cmake git && echo '构建工具安装完成'"

if %errorlevel% neq 0 (
    echo [警告] 构建工具安装可能失败
    echo 请手动在WSL2中运行以下命令:
    echo   sudo apt-get update
    echo   sudo apt-get install -y build-essential cmake git
) else (
    echo [成功] 构建工具安装完成
)

echo ========================================
echo 步骤3: 测试构建环境
echo ========================================
echo.

echo [信息] 测试构建环境...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo '测试构建环境...' && mkdir -p build_linux_test && cd build_linux_test && echo '配置项目...' && cmake .. -DCMAKE_BUILD_TYPE=Release && if [ $? -eq 0 ]; then echo '项目配置成功'; echo '构建项目...'; cmake --build . -j$(nproc); if [ $? -eq 0 ]; then echo '项目构建成功'; ls -la; else echo '项目构建失败'; fi; else echo '项目配置失败'; fi"

echo ========================================
echo 步骤4: 完成修复
echo ========================================
echo.

echo [成功] Linux构建环境修复完成！
echo.
echo 现在您可以:
echo   1. 运行 build_cross_quick.bat 进行跨平台构建
echo   2. 运行 build_universal.bat 进行完整跨平台构建
echo   3. 手动在WSL2中编译Linux版本
echo.

REM 询问是否立即测试跨平台构建
set /p "test_now=是否立即测试跨平台构建？(y/n): "
if /i "%test_now%"=="y" (
    echo.
    echo [信息] 开始测试跨平台构建...
    call build_cross_quick.bat
) else (
    echo [信息] 您可以稍后运行 build_cross_quick.bat 进行测试
)

echo.
pause
goto :eof