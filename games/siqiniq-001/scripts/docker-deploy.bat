@echo off
REM Docker deployment script for Siqiniq-001

setlocal

echo ========================================
echo Siqiniq-001 Docker Deployment
echo ========================================
echo.

set PROFILE=default
set BUILD=false

:parse_args
if "%1"=="" goto done_args
if /i "%1"=="--build" set BUILD=true
if /i "%1"=="--monitoring" set PROFILE=monitoring
if /i "%1"=="--admin" set PROFILE=admin
if /i "%1"=="--all" set PROFILE=monitoring,admin
shift
goto parse_args

:done_args

echo Configuration:
echo   Profile: %PROFILE%
echo   Build: %BUILD%
echo.

REM Build if requested
if "%BUILD%"=="true" (
    echo Building application...
    call mvnw.cmd clean package -DskipTests
    if %ERRORLEVEL% NEQ 0 (
        echo Build failed!
        exit /b 1
    )
    echo.
)

REM Deploy with Docker Compose
cd docker

if "%PROFILE%"=="default" (
    echo Deploying core services...
    docker-compose up -d mariadb redis siqiniq-server
) else (
    echo Deploying with profiles: %PROFILE%
    docker-compose --profile %PROFILE% up -d
)

if %ERRORLEVEL% NEQ 0 (
    echo Deployment failed!
    exit /b 1
)

echo.
echo ========================================
echo Deployment Complete!
echo ========================================
echo.
echo Services:
echo   Game Server:    http://localhost:8080
echo   API Docs:       http://localhost:8080/swagger-ui
echo   Health Check:   http://localhost:8080/q/health
echo.

if "%PROFILE%"=="admin" (
    echo Admin Tools:
    echo   phpMyAdmin:     http://localhost:8081
    echo   Redis Commander: http://localhost:8082
    echo.
)

if "%PROFILE%"=="monitoring" (
    echo Monitoring:
    echo   Prometheus:     http://localhost:9090
    echo   Grafana:        http://localhost:3000
    echo.
)

echo To view logs:
echo   docker-compose logs -f
echo.
echo To stop:
echo   docker-compose down
echo.

endlocal
