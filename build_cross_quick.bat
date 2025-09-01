@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo SwiftPeer P2P Core - 快速跨平台构建
echo ========================================
echo 目标: 快速生成.dll和.so文件
echo.

REM 检查CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 未找到CMake，请先安装CMake
    echo 下载地址: https://cmake.org/download/
    pause
    exit /b 1
)

echo [信息] 开始跨平台构建...
echo.

REM 检查跨平台环境
set "CROSS_ENV="
where wsl >nul 2>nul
if %errorlevel% equ 0 (
    set "CROSS_ENV=WSL2"
    echo [信息] 使用WSL2进行跨平台编译
) else (
    where docker >nul 2>nul
    if %errorlevel% equ 0 (
        set "CROSS_ENV=Docker"
        echo [信息] 使用Docker进行跨平台编译
    ) else (
        echo [警告] 未找到跨平台编译环境
        echo 将仅构建Windows版本
        echo 如需跨平台编译，请安装WSL2或Docker
        echo.
    )
)

REM 第一步: 构建Windows版本
echo ========================================
echo 步骤1: 构建Windows版本 (.dll)
echo ========================================
echo.

REM 创建Windows构建目录
if exist build_cross_windows (
    echo [信息] 清理旧的Windows构建目录...
    rmdir /s /q build_cross_windows
)
mkdir build_cross_windows
cd build_cross_windows

REM 配置Windows项目
echo [信息] 配置Windows项目...
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/utf-8"
if %errorlevel% neq 0 (
    echo [错误] Windows项目配置失败
    cd ..
    pause
    exit /b 1
)

REM 构建Windows项目
echo [信息] 构建Windows项目...
cmake --build . --config Release -j4
if %errorlevel% neq 0 (
    echo [错误] Windows项目构建失败
    cd ..
    pause
    exit /b 1
)

cd ..

REM 第二步: 构建Linux版本 (如果有跨平台环境)
if not "%CROSS_ENV%"=="" (
    echo.
    echo ========================================
    echo 步骤2: 构建Linux版本 (.so)
    echo ========================================
    echo.

    if "%CROSS_ENV%"=="WSL2" (
        call :build_linux_wsl
    ) else if "%CROSS_ENV%"=="Docker" (
        call :build_linux_docker
    )
)

REM 显示最终结果
echo.
echo ========================================
echo [成功] 跨平台构建完成！
echo ========================================
echo.
echo 生成的库文件:
echo.

REM Windows版本
echo Windows版本:
if exist "build_cross_windows\bin\Release\p2p_core.dll" (
    echo   DLL: build_cross_windows\bin\Release\p2p_core.dll
    echo   导入库: build_cross_windows\lib\Release\p2p_core.lib
) else if exist "build_cross_windows\src\p2p_core.dll" (
    echo   DLL: build_cross_windows\src\p2p_core.dll
    echo   导入库: build_cross_windows\src\p2p_core.lib
) else (
    echo   警告: 未找到Windows库文件
)
echo.

REM Linux版本
if not "%CROSS_ENV%"=="" (
    echo Linux版本:
    if exist "build_cross_linux\src\libp2p_core.so" (
        echo   共享库: build_cross_linux\src\libp2p_core.so
    ) else (
        echo   警告: 未找到Linux库文件
    )
    echo.
)

echo 头文件位置: include\p2p\
echo.

if not "%CROSS_ENV%"=="" (
    echo 现在您拥有了两个平台的库文件！
    echo.
    echo 使用方法:
    echo   1. Windows项目: 链接p2p_core.lib，运行时需要p2p_core.dll
    echo   2. Linux项目: 链接libp2p_core.so
    echo   3. 头文件: 两个平台都可以使用include\p2p\目录
) else (
    echo 仅生成了Windows版本库文件
    echo 如需Linux版本，请安装WSL2或Docker后重新运行
)

echo.
pause
goto :eof

REM 使用WSL2构建Linux版本
:build_linux_wsl
echo [信息] 使用WSL2构建Linux版本...
echo.

REM 创建Linux构建目录
if exist build_cross_linux (
    echo [信息] 清理旧的Linux构建目录...
    rmdir /s /q build_cross_linux
)

REM 在WSL2中构建
echo [信息] 在WSL2中执行构建...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo '在WSL2中构建Linux版本...' && if ! command -v cmake >/dev/null 2>&1; then echo '安装CMake...'; sudo apt-get update; sudo apt-get install -y cmake build-essential; fi && mkdir -p build_cross_linux && cd build_cross_linux && echo '配置Linux项目...' && cmake .. -DCMAKE_BUILD_TYPE=Release && echo '构建Linux项目...' && cmake --build . -j$(nproc) && echo 'Linux版本构建完成'"

if %errorlevel% neq 0 (
    echo [警告] WSL2构建可能失败，请检查WSL2环境
) else (
    echo [成功] Linux版本构建完成
)
goto :eof

REM 使用Docker构建Linux版本
:build_linux_docker
echo [信息] 使用Docker构建Linux版本...
echo.

REM 创建Linux构建目录
if exist build_cross_linux (
    echo [信息] 清理旧的Linux构建目录...
    rmdir /s /q build_cross_linux
)

REM 在Docker中构建
echo [信息] 在Docker中执行构建...
docker run --rm -v "%cd%:/workspace" -w /workspace ubuntu:20.04 bash -c "echo '在Docker中构建Linux版本...' && echo '安装构建依赖...' && apt-get update && apt-get install -y cmake build-essential && mkdir -p build_cross_linux && cd build_cross_linux && echo '配置Linux项目...' && cmake .. -DCMAKE_BUILD_TYPE=Release && echo '构建Linux项目...' && cmake --build . -j$(nproc) && echo 'Linux版本构建完成'"

if %errorlevel% neq 0 (
    echo [警告] Docker构建可能失败，请检查Docker环境
) else (
    echo [成功] Linux版本构建完成
)
goto :eof