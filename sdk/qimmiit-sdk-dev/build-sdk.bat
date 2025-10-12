@echo off
REM Quick build script for Qimmiit SDK Dev

echo ========================================
echo Qimmiit SDK Dev - Build Script
echo ========================================
echo.

REM Check if vcpkg is available
if not defined VCPKG_ROOT (
    if exist "C:\vcpkg" (
        set VCPKG_ROOT=C:\vcpkg
        echo Using vcpkg from: %VCPKG_ROOT%
    ) else (
        echo ERROR: VCPKG_ROOT not set and C:\vcpkg not found
        echo Please set VCPKG_ROOT environment variable
        echo Example: set VCPKG_ROOT=C:\path\to\vcpkg
        exit /b 1
    )
) else (
    echo Using vcpkg from: %VCPKG_ROOT%
)

echo.
echo Configuring SDK...
cmake -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Configuration FAILED
    exit /b 1
)

echo.
echo ========================================
echo Build configured successfully!
echo.
echo To build:
echo   cmake --build build --config Release
echo.
echo SDK includes:
echo   - coregfx (rendering)
echo   - cryo-asset (assets)
echo   - cryo-cache (caching)
echo   - akutik (storage)
echo   - cryo-tooling (tools)
echo ========================================
