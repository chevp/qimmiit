@echo off
REM Build script for Avannaaq-001 (Windows)

setlocal

echo ========================================
echo Avannaaq-001 Build Script
echo ========================================
echo.

REM Check for vcpkg
if not defined VCPKG_ROOT (
    if exist "C:\vcpkg" (
        set VCPKG_ROOT=C:\vcpkg
    ) else (
        echo ERROR: VCPKG_ROOT not set
        echo Please set VCPKG_ROOT environment variable
        exit /b 1
    )
)

echo Using vcpkg from: %VCPKG_ROOT%
echo.

REM Parse arguments
set BUILD_TYPE=Release
set BUILD_SERVER=ON
set BUILD_CLIENT=ON
set BUILD_CMS=ON

:parse_args
if "%1"=="" goto done_args
if /i "%1"=="--debug" set BUILD_TYPE=Debug
if /i "%1"=="--no-server" set BUILD_SERVER=OFF
if /i "%1"=="--no-client" set BUILD_CLIENT=OFF
if /i "%1"=="--no-cms" set BUILD_CMS=OFF
shift
goto parse_args

:done_args

echo Configuration:
echo   Build Type: %BUILD_TYPE%
echo   Build Server: %BUILD_SERVER%
echo   Build Client: %BUILD_CLIENT%
echo   Build CMS: %BUILD_CMS%
echo.

REM Configure
echo Configuring...
cmake -B build ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DAVANNAAQ_BUILD_SERVER=%BUILD_SERVER% ^
    -DAVANNAAQ_BUILD_CLIENT=%BUILD_CLIENT% ^
    -DAVANNAAQ_BUILD_CMS=%BUILD_CMS%

if %ERRORLEVEL% NEQ 0 (
    echo Configuration failed!
    exit /b 1
)

echo.
echo Building...
cmake --build build --config %BUILD_TYPE% -j

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Binaries located in: build\bin\
echo.
echo To run the server:
echo   cd build\bin
echo   avannaaq-server.exe --config ..\..\config\server_dev.json
echo.

endlocal
