@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo SwiftPeer P2P Core - WSL2环境设置脚本
echo ========================================
echo 目标: 自动安装和配置WSL2环境
echo.

echo [信息] 开始设置WSL2环境...
echo.

REM 检查管理员权限
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 需要管理员权限来安装WSL2
    echo 请以管理员身份运行此脚本
    pause
    exit /b 1
)

echo [信息] 检查WSL2状态...
wsl --status >nul 2>&1
if %errorlevel% equ 0 (
    echo [信息] WSL2已安装
) else (
    echo [信息] 安装WSL2...
    wsl --install
    if %errorlevel% neq 0 (
        echo [错误] WSL2安装失败
        pause
        exit /b 1
    )
    echo [信息] WSL2安装完成，请重启计算机
    echo 重启后请重新运行此脚本
    pause
    exit /b 0
)

echo [信息] 检查Ubuntu发行版...
wsl --list --quiet | findstr "Ubuntu" >nul 2>&1
if %errorlevel% neq 0 (
    echo [信息] 安装Ubuntu 22.04 LTS...
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
    echo [信息] Ubuntu已安装
)

echo [信息] 安装构建工具...
wsl bash -c "echo '安装构建工具...' && sudo apt-get update && sudo apt-get install -y build-essential cmake git && echo '构建工具安装完成'"

if %errorlevel% neq 0 (
    echo [警告] 构建工具安装可能失败
    echo 请手动在WSL2中运行以下命令:
    echo   sudo apt-get update
    echo   sudo apt-get install -y build-essential cmake git
) else (
    echo [成功] 构建工具安装完成
)

echo [信息] 测试WSL2环境...
wsl bash -c "echo '测试WSL2环境...' && echo 'CMake版本:'; cmake --version && echo 'GCC版本:'; gcc --version && echo 'Git版本:'; git --version"

echo ========================================
echo 设置完成
echo ========================================
echo.

echo [成功] WSL2环境设置完成！
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