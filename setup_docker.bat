@echo off
echo ========================================
echo SwiftPeer P2P Core - Docker环境设置脚本
echo ========================================
echo 目标: 快速配置Docker环境用于跨平台编译
echo.

REM 检查Docker是否已安装
where docker >nul 2>nul
if %errorlevel% equ 0 (
    echo [信息] 检测到Docker已安装
    echo.
    
    REM 检查Docker状态
    echo [信息] 检查Docker状态...
    docker --version
    docker info --format "Docker版本: {{.ServerVersion}}"
    echo.
    
    REM 检查Docker是否运行
    docker ps >nul 2>&1
    if %errorlevel% neq 0 (
        echo [警告] Docker服务未运行
        echo 请启动Docker Desktop应用程序
        echo.
        set /p "start_docker=是否现在启动Docker？(y/n): "
        if /i "%start_docker%"=="y" (
            echo [信息] 请手动启动Docker Desktop
            echo 启动完成后，请重新运行此脚本
            pause
            exit /b 0
        )
    ) else (
        echo [成功] Docker服务正在运行
        goto :test_docker
    )
) else (
    echo [信息] 未检测到Docker，开始安装指导...
    goto :install_docker
)

:install_docker
echo ========================================
echo 步骤1: 安装Docker Desktop
echo ========================================
echo.

echo [信息] 请按照以下步骤安装Docker Desktop:
echo.
echo 1. 下载Docker Desktop for Windows:
echo    https://www.docker.com/products/docker-desktop
echo.
echo 2. 运行安装程序，选择"Use WSL 2 instead of Hyper-V"
echo.
echo 3. 安装完成后重启计算机
echo.
echo 4. 启动Docker Desktop应用程序
echo.
echo 5. 等待Docker服务完全启动
echo.
echo 6. 重新运行此脚本继续配置
echo.

set /p "download=是否现在打开下载页面？(y/n): "
if /i "%download%"=="y" (
    start https://www.docker.com/products/docker-desktop
)

echo.
echo 安装完成后，请重新运行此脚本
pause
exit /b 0

:test_docker
echo ========================================
echo 步骤2: 测试Docker环境
echo ========================================
echo.

echo [信息] 测试Docker环境...
echo.

REM 测试Docker基本功能
echo [信息] 测试Docker基本功能...
docker run --rm hello-world
if %errorlevel% neq 0 (
    echo [错误] Docker基本功能测试失败
    echo 请检查Docker安装和配置
    pause
    exit /b 1
)

echo [成功] Docker基本功能测试通过
echo.

REM 测试Ubuntu镜像
echo [信息] 测试Ubuntu镜像...
docker run --rm ubuntu:20.04 echo "Ubuntu镜像测试成功"
if %errorlevel% neq 0 (
    echo [错误] Ubuntu镜像测试失败
    echo 请检查网络连接和Docker配置
    pause
    exit /b 1
)

echo [成功] Ubuntu镜像测试通过
echo.

echo ========================================
echo 步骤3: 配置Docker构建环境
echo ========================================
echo.

echo [信息] 配置Docker构建环境...
echo.

REM 创建Dockerfile用于构建
echo [信息] 创建专用Dockerfile...
if exist Dockerfile.build (
    del Dockerfile.build
)

REM 创建优化的构建Dockerfile
echo FROM ubuntu:20.04 > Dockerfile.build
echo. >> Dockerfile.build
echo # 设置环境变量 >> Dockerfile.build
echo ENV DEBIAN_FRONTEND=noninteractive >> Dockerfile.build
echo ENV TZ=Asia/Shanghai >> Dockerfile.build
echo. >> Dockerfile.build
echo # 安装构建工具 >> Dockerfile.build
echo RUN apt-get update ^&^& apt-get install -y \ >> Dockerfile.build
echo     build-essential \ >> Dockerfile.build
echo     cmake \ >> Dockerfile.build
echo     git \ >> Dockerfile.build
echo     curl \ >> Dockerfile.build
echo     wget \ >> Dockerfile.build
echo     unzip \ >> Dockerfile.build
echo     ^&^& apt-get clean \ >> Dockerfile.build
echo     ^&^& rm -rf /var/lib/apt/lists/* >> Dockerfile.build
echo. >> Dockerfile.build
echo # 设置工作目录 >> Dockerfile.build
echo WORKDIR /workspace >> Dockerfile.build
echo. >> Dockerfile.build
echo # 设置构建脚本 >> Dockerfile.build
echo COPY build_linux.sh /usr/local/bin/ >> Dockerfile.build
echo RUN chmod +x /usr/local/bin/build_linux.sh >> Dockerfile.build
echo. >> Dockerfile.build
echo # 默认命令 >> Dockerfile.build
echo CMD ["/bin/bash"] >> Dockerfile.build

echo [成功] Dockerfile.build创建完成
echo.

REM 创建Linux构建脚本
echo [信息] 创建Linux构建脚本...
if exist build_linux.sh (
    del build_linux.sh
)

echo #!/bin/bash > build_linux.sh
echo set -e >> build_linux.sh
echo. >> build_linux.sh
echo echo "开始构建Linux版本..." >> build_linux.sh
echo. >> build_linux.sh
echo # 检查依赖 >> build_linux.sh
echo if ! command -v cmake ^&^> /dev/null; then >> build_linux.sh
echo     echo "安装CMake..." >> build_linux.sh
echo     apt-get update >> build_linux.sh
echo     apt-get install -y cmake build-essential >> build_linux.sh
echo fi >> build_linux.sh
echo. >> build_linux.sh
echo # 创建构建目录 >> build_linux.sh
echo mkdir -p build_linux >> build_linux.sh
echo cd build_linux >> build_linux.sh
echo. >> build_linux.sh
echo # 配置项目 >> build_linux.sh
echo echo "配置Linux项目..." >> build_linux.sh
echo cmake .. -DCMAKE_BUILD_TYPE=Release >> build_linux.sh
echo. >> build_linux.sh
echo # 构建项目 >> build_linux.sh
echo echo "构建Linux项目..." >> build_linux.sh
echo cmake --build . -j$(nproc) >> build_linux.sh
echo. >> build_linux.sh
echo echo "Linux版本构建完成" >> build_linux.sh
echo ls -la >> build_linux.sh

echo [成功] build_linux.sh创建完成
echo.

REM 构建Docker镜像
echo [信息] 构建专用Docker镜像...
docker build -f Dockerfile.build -t swiftpeer-builder .
if %errorlevel% neq 0 (
    echo [错误] Docker镜像构建失败
    echo 请检查Dockerfile和网络连接
    pause
    exit /b 1
)

echo [成功] Docker镜像构建完成
echo.

echo ========================================
echo 步骤4: 测试跨平台编译
echo ========================================
echo.

echo [信息] 测试Docker跨平台编译...
echo.

REM 创建测试目录
if exist test_docker_compile (
    rmdir /s /q test_docker_compile
)
mkdir test_docker_compile
cd test_docker_compile

REM 创建简单的测试CMakeLists.txt
echo cmake_minimum_required(VERSION 3.16) > CMakeLists.txt
echo project(TestDockerCompile) >> CMakeLists.txt
echo add_library(test_lib SHARED test.cpp) >> CMakeLists.txt
echo set_target_properties(test_lib PROPERTIES OUTPUT_NAME "test_lib") >> CMakeLists.txt

REM 创建测试源文件
echo #include ^<iostream^> > test.cpp
echo int main() { std::cout ^<^< "Hello from Docker Linux!" ^<^< std::endl; return 0; } >> test.cpp

REM 在Docker中测试编译
echo [信息] 在Docker中测试编译...
docker run --rm -v "%cd%:/workspace" swiftpeer-builder bash -c "
    echo '测试Docker Linux编译...'
    
    mkdir -p build_test
    cd build_test
    
    cmake ..
    if [ $? -eq 0 ]; then
        cmake --build .
        if [ $? -eq 0 ]; then
            echo 'Docker Linux编译测试成功！'
            ls -la
        else
            echo 'Docker Linux编译测试失败'
        fi
    else
        echo 'Docker Linux配置测试失败'
    fi
"

cd ..

echo ========================================
echo 步骤5: 完成配置
echo ========================================
echo.

echo [成功] Docker环境配置完成！
echo.
echo 现在您可以:
echo   1. 运行 build_cross_quick.bat 进行跨平台构建
echo   2. 运行 build_universal.bat 进行完整跨平台构建
echo   3. 使用Docker手动编译Linux版本
echo.
echo 使用方法:
echo   - 在Windows中: 使用Visual Studio编译.dll文件
echo   - 在Docker中: 使用GCC编译.so文件
echo   - 两个平台共享相同的源代码和头文件
echo.
echo Docker命令示例:
echo   docker run --rm -v "%cd%:/workspace" swiftpeer-builder bash -c "cd /workspace ^&^& mkdir -p build_linux ^&^& cd build_linux ^&^& cmake .. ^&^& cmake --build ."
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
