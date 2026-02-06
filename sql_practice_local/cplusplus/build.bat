@echo off
REM Build script for SQL Practice C++ Server on Windows with Clang

echo ========================================
echo SQL Practice C++ Server - Build Script
echo ========================================
echo.

REM Check if Clang is installed
where clang >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Clang not found in PATH
    echo Please install LLVM from: https://github.com/llvm/llvm-project/releases
    echo.
    pause
    exit /b 1
)

REM Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake from: https://cmake.org/download/
    echo.
    pause
    exit /b 1
)

echo [1/3] Tools found:
clang --version
cmake --version
echo.

echo [2/3] Configuring build...
if not exist build mkdir build
cd build

REM Try Ninja first (faster), fall back to Visual Studio generator
where ninja >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Using Ninja generator...
    cmake .. -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
) else (
    echo Using Visual Studio generator with ClangCL...
    cmake .. -G "Visual Studio 17 2022" -A x64 -T ClangCL
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo.
echo [3/3] Building...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo To run the server:
echo   cd build
echo   Release\sql-practice-server.exe
echo.
echo Or from current directory:
echo   build\Release\sql-practice-server.exe
echo.

pause
