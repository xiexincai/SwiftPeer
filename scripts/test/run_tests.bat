@echo off
REM Windows测试脚本

set BUILD_DIR=build
set TEST_DIR=tests

if not exist %BUILD_DIR% (
    echo 请先构建项目�?
    pause
    exit /b 1
)

cd %BUILD_DIR%

REM 运行单元测试
echo 运行单元测试...
ctest -C Release --output-on-failure

REM 运行性能测试
echo 运行性能测试...
if exist %TEST_DIR%\Release\performance_tests.exe (
    %TEST_DIR%\Release\performance_tests.exe
)

echo 测试完成�?
pause
