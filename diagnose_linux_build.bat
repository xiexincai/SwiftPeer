@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo SwiftPeer P2P Core - Linux构建诊断脚本
echo ========================================
echo 目标: 诊断和修复Linux版本构建问题
echo.

echo [信息] 开始诊断Linux构建环境...
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

echo [信息] 检查WSL2状态...
wsl --status
echo.

echo [信息] 检查已安装的Linux发行版...
wsl --list --verbose
echo.

REM 检查Ubuntu发行版
wsl --list --quiet | findstr "Ubuntu" >nul 2>&1
if %errorlevel% neq 0 (
    echo [警告] 未找到Ubuntu发行版
    echo 建议安装Ubuntu 22.04 LTS
    echo 运行: wsl --install -d Ubuntu-22.04
    echo.
) else (
    echo [成功] 找到Ubuntu发行版
)

echo ========================================
echo 步骤2: 检查Linux构建工具
echo ========================================
echo.

echo [信息] 检查Linux构建工具...
wsl bash -c "echo '检查构建工具...'; echo 'CMake:'; which cmake; echo 'GCC:'; which gcc; echo 'G++:'; which g++; echo 'Make:'; which make; echo 'Git:'; which git"

echo ========================================
echo 步骤3: 检查项目路径
echo ========================================
echo.

echo [信息] 检查项目路径...
wsl bash -c "echo '检查项目路径...'; if [ -d '/mnt/d/Develop/Project/SwiftPeer' ]; then echo '项目目录存在: /mnt/d/Develop/Project/SwiftPeer'; echo '项目目录内容:'; ls -la /mnt/d/Develop/Project/SwiftPeer; else echo '项目目录不存在: /mnt/d/Develop/Project/SwiftPeer'; fi"

echo ========================================
echo 步骤4: 检查项目文件
echo ========================================
echo.

echo [信息] 检查项目文件...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo '检查项目文件...'; if [ -f 'CMakeLists.txt' ]; then echo 'CMakeLists.txt存在'; else echo 'CMakeLists.txt不存在'; fi; if [ -d 'src' ]; then echo '源代码目录存在'; else echo '源代码目录不存在'; fi; if [ -d 'include' ]; then echo '头文件目录存在'; else echo '头文件目录不存在'; fi"

echo ========================================
echo 步骤5: 测试简单编译
echo ========================================
echo.

echo [信息] 测试简单编译...
wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo '测试简单编译...'; mkdir -p test_build && cd test_build && echo 'int main(){return 0;}' > test.cpp && g++ -o test test.cpp && if [ -f 'test' ]; then echo '简单编译测试成功'; else echo '简单编译测试失败'; fi"

echo ========================================
echo 步骤6: 修复建议
echo ========================================
echo.

echo [信息] 根据诊断结果提供修复建议...
echo.

echo 如果遇到以下问题，请按以下步骤修复:
echo.
echo 1. WSL2未安装:
echo    - 运行: wsl --install
echo    - 重启计算机
echo    - 重新运行此脚本
echo.
echo 2. Ubuntu未安装:
echo    - 运行: wsl --install -d Ubuntu-22.04
echo    - 设置用户名和密码
echo    - 重新运行此脚本
echo.
echo 3. 构建工具未安装:
echo    - 在WSL2中运行: sudo apt-get update
echo    - 运行: sudo apt-get install -y build-essential cmake git
echo    - 重新运行此脚本
echo.
echo 4. 项目路径问题:
echo    - 确保项目位于: D:\Develop\Project\SwiftPeer
echo    - 检查CMakeLists.txt是否存在
echo    - 检查源代码和头文件目录
echo.
echo 5. 权限问题:
echo    - 确保WSL2有访问Windows文件的权限
echo    - 检查文件权限设置
echo.

echo ========================================
echo 诊断完成
echo ========================================
echo.

echo [信息] 诊断完成！请根据上述建议修复问题。
echo 修复后，请重新运行 build_cross_quick.bat
echo.

pause
goto :eof