@echo off
REM Build script for Siqiniq-001

setlocal

echo ========================================
echo Siqiniq-001 Build Script
echo ========================================
echo.

set BUILD_TYPE=jvm
set SKIP_TESTS=false

:parse_args
if "%1"=="" goto done_args
if /i "%1"=="--native" set BUILD_TYPE=native
if /i "%1"=="--skip-tests" set SKIP_TESTS=true
shift
goto parse_args

:done_args

echo Configuration:
echo   Build Type: %BUILD_TYPE%
echo   Skip Tests: %SKIP_TESTS%
echo.

if "%BUILD_TYPE%"=="native" (
    echo Building native image...
    echo This requires GraalVM and may take several minutes...
    echo.

    if "%SKIP_TESTS%"=="true" (
        call mvnw.cmd clean package -Pnative -DskipTests
    ) else (
        call mvnw.cmd clean package -Pnative
    )
) else (
    echo Building JVM application...
    echo.

    if "%SKIP_TESTS%"=="true" (
        call mvnw.cmd clean package -DskipTests
    ) else (
        call mvnw.cmd clean package
    )
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed!
    exit /b 1
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.

if "%BUILD_TYPE%"=="native" (
    echo Native executable: target\siqiniq-001-runner.exe
    echo.
    echo To run:
    echo   .\target\siqiniq-001-runner.exe
) else (
    echo JAR file: target\quarkus-app\quarkus-run.jar
    echo.
    echo To run:
    echo   java -jar target\quarkus-app\quarkus-run.jar
)

echo.

endlocal
