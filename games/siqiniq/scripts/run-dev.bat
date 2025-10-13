@echo off
REM Run Siqiniq-001 in development mode

echo ========================================
echo Siqiniq-001 - Development Mode
echo ========================================
echo.
echo Starting Quarkus in dev mode...
echo Dev UI will be available at: http://localhost:8080/q/dev/
echo.

call mvnw.cmd quarkus:dev

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Failed to start dev server
    pause
    exit /b 1
)
