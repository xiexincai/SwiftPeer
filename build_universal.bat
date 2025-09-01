@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo SwiftPeer P2P Core - Universal Cross-Platform Build
echo ========================================
echo Target: Complete cross-platform build with multiple configurations
echo.

REM Parse command line arguments
set "BUILD_TYPE=Release"
set "CLEAN_BUILD=false"
set "VERBOSE=false"
set "PARALLEL_JOBS=4"

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--debug" set "BUILD_TYPE=Debug"
if /i "%~1"=="--release" set "BUILD_TYPE=Release"
if /i "%~1"=="--clean" set "CLEAN_BUILD=true"
if /i "%~1"=="--verbose" set "VERBOSE=true"
if /i "%~1"=="--jobs" (
    shift
    set "PARALLEL_JOBS=%~1"
)
if /i "%~1"=="--help" goto :show_usage
shift
goto :parse_args

:args_done

echo [INFO] Build Configuration:
echo   - Build Type: %BUILD_TYPE%
echo   - Clean Build: %CLEAN_BUILD%
echo   - Verbose Output: %VERBOSE%
echo   - Parallel Jobs: %PARALLEL_JOBS%
echo.

REM Check CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found, please install CMake first
    echo Download: https://cmake.org/download/
    pause
    exit /b 1
)

echo [INFO] Starting universal cross-platform build...
echo.

REM Check cross-platform environment
set "CROSS_ENV="
where wsl >nul 2>nul
if %errorlevel% equ 0 (
    set "CROSS_ENV=WSL2"
    echo [INFO] Using WSL2 for cross-platform compilation
) else (
    where docker >nul 2>nul
    if %errorlevel% equ 0 (
        set "CROSS_ENV=Docker"
        echo [INFO] Using Docker for cross-platform compilation
    ) else (
        echo [WARNING] Cross-platform compilation environment not found
        echo Will only build Windows version
        echo To enable cross-platform compilation, please install WSL2 or Docker
        echo.
    )
)

REM Clean build directories
if "%CLEAN_BUILD%"=="true" (
    echo [INFO] Cleaning build directories...
    if exist build_universal_windows rmdir /s /q build_universal_windows
    if exist build_universal_linux rmdir /s /q build_universal_linux
)

REM Step 1: Build Windows version
echo ========================================
echo Step 1: Building Windows version (.dll)
echo ========================================
echo.

REM Create Windows build directory
if exist build_universal_windows (
    echo [INFO] Cleaning old Windows build directory...
    rmdir /s /q build_universal_windows
)
mkdir build_universal_windows
cd build_universal_windows

REM Configure Windows project
echo [INFO] Configuring Windows project...
if "%VERBOSE%"=="true" (
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_CXX_FLAGS="/utf-8" -DCMAKE_VERBOSE_MAKEFILE=ON
) else (
    cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_CXX_FLAGS="/utf-8"
)
if %errorlevel% neq 0 (
    echo [ERROR] Windows project configuration failed
    cd ..
    pause
    exit /b 1
)

REM Build Windows project
echo [INFO] Building Windows project...
if "%VERBOSE%"=="true" (
    cmake --build . --config %BUILD_TYPE% --verbose -j%PARALLEL_JOBS%
) else (
    cmake --build . --config %BUILD_TYPE% -j%PARALLEL_JOBS%
)
if %errorlevel% neq 0 (
    echo [ERROR] Windows project build failed
    cd ..
    pause
    exit /b 1
)

cd ..

REM Step 2: Build Linux version (if cross-platform environment available)
if not "%CROSS_ENV%"=="" (
    echo.
    echo ========================================
    echo Step 2: Building Linux version (.so)
    echo ========================================
    echo.

    if "%CROSS_ENV%"=="WSL2" (
        call :build_linux_wsl
    ) else if "%CROSS_ENV%"=="Docker" (
        call :build_linux_docker
    )
)

REM Show final results
echo.
echo ========================================
echo [SUCCESS] Universal cross-platform build completed!
echo ========================================
echo.
echo Generated library files:
echo.

REM Windows version
echo Windows version:
if exist "build_universal_windows\bin\%BUILD_TYPE%\p2p_core.dll" (
    echo   DLL: build_universal_windows\bin\%BUILD_TYPE%\p2p_core.dll
    echo   Import Library: build_universal_windows\lib\%BUILD_TYPE%\p2p_core.lib
) else if exist "build_universal_windows\src\p2p_core.dll" (
    echo   DLL: build_universal_windows\src\p2p_core.dll
    echo   Import Library: build_universal_windows\src\p2p_core.lib
) else (
    echo   WARNING: Windows library files not found
)
echo.

REM Linux version
if not "%CROSS_ENV%"=="" (
    echo Linux version:
    if exist "build_universal_linux\src\libp2p_core.so" (
        echo   Shared Library: build_universal_linux\src\libp2p_core.so
    ) else (
        echo   WARNING: Linux library files not found
    )
    echo.
)

echo Header files location: include\p2p\
echo.

if not "%CROSS_ENV%"=="" (
    echo Now you have library files for both platforms!
    echo.
    echo Usage:
    echo   1. Windows projects: Link p2p_core.lib, runtime requires p2p_core.dll
    echo   2. Linux projects: Link libp2p_core.so
    echo   3. Header files: Both platforms can use include\p2p\ directory
) else (
    echo Only Windows version library files generated
    echo To get Linux version, please install WSL2 or Docker and run again
)

echo.
pause
goto :eof

REM Build Linux version using WSL2
:build_linux_wsl
echo [INFO] Building Linux version using WSL2...
echo.

REM Create Linux build directory
if exist build_universal_linux (
    echo [INFO] Cleaning old Linux build directory...
    rmdir /s /q build_universal_linux
)

REM Build in WSL2
echo [INFO] Executing build in WSL2...
if "%BUILD_TYPE%"=="Debug" (
    if "%VERBOSE%"=="true" (
        wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo 'Building Linux version in WSL2...' && if ! command -v cmake >/dev/null 2>&1; then echo 'Installing CMake...'; sudo apt-get update; sudo apt-get install -y cmake build-essential; fi && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    ) else (
        wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo 'Building Linux version in WSL2...' && if ! command -v cmake >/dev/null 2>&1; then echo 'Installing CMake...'; sudo apt-get update; sudo apt-get install -y cmake build-essential; fi && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Debug && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    )
) else (
    if "%VERBOSE%"=="true" (
        wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo 'Building Linux version in WSL2...' && if ! command -v cmake >/dev/null 2>&1; then echo 'Installing CMake...'; sudo apt-get update; sudo apt-get install -y cmake build-essential; fi && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=ON && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    ) else (
        wsl bash -c "cd /mnt/d/Develop/Project/SwiftPeer && echo 'Building Linux version in WSL2...' && if ! command -v cmake >/dev/null 2>&1; then echo 'Installing CMake...'; sudo apt-get update; sudo apt-get install -y cmake build-essential; fi && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Release && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    )
)

if %errorlevel% neq 0 (
    echo [WARNING] WSL2 build may have failed, please check WSL2 environment
) else (
    echo [SUCCESS] Linux version build completed
)
goto :eof

REM Build Linux version using Docker
:build_linux_docker
echo [INFO] Building Linux version using Docker...
echo.

REM Create Linux build directory
if exist build_universal_linux (
    echo [INFO] Cleaning old Linux build directory...
    rmdir /s /q build_universal_linux
)

REM Build in Docker
echo [INFO] Executing build in Docker...
if "%BUILD_TYPE%"=="Debug" (
    if "%VERBOSE%"=="true" (
        docker run --rm -v "%cd%:/workspace" -w /workspace ubuntu:20.04 bash -c "echo 'Building Linux version in Docker...' && echo 'Installing build dependencies...' && apt-get update && apt-get install -y cmake build-essential && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    ) else (
        docker run --rm -v "%cd%:/workspace" -w /workspace ubuntu:20.04 bash -c "echo 'Building Linux version in Docker...' && echo 'Installing build dependencies...' && apt-get update && apt-get install -y cmake build-essential && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Debug && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    )
) else (
    if "%VERBOSE%"=="true" (
        docker run --rm -v "%cd%:/workspace" -w /workspace ubuntu:20.04 bash -c "echo 'Building Linux version in Docker...' && echo 'Installing build dependencies...' && apt-get update && apt-get install -y cmake build-essential && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=ON && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    ) else (
        docker run --rm -v "%cd%:/workspace" -w /workspace ubuntu:20.04 bash -c "echo 'Building Linux version in Docker...' && echo 'Installing build dependencies...' && apt-get update && apt-get install -y cmake build-essential && mkdir -p build_universal_linux && cd build_universal_linux && echo 'Configuring Linux project...' && cmake .. -DCMAKE_BUILD_TYPE=Release && echo 'Building Linux project...' && cmake --build . -j$(nproc) && echo 'Linux build completed'"
    )
)

if %errorlevel% neq 0 (
    echo [WARNING] Docker build may have failed, please check Docker environment
) else (
    echo [SUCCESS] Linux version build completed
)
goto :eof

REM Show usage help
:show_usage
echo Usage: build_universal.bat [options]
echo.
echo Options:
echo   --debug      Build debug version
echo   --release    Build release version (default)
echo   --clean      Clean build directories
echo   --verbose    Verbose output
echo   --jobs N     Number of parallel jobs (default: 4)
echo   --help       Show this help information
echo.
echo Examples:
echo   build_universal.bat --release --clean
echo   build_universal.bat --debug --verbose --jobs 8
echo.
pause
exit /b 0