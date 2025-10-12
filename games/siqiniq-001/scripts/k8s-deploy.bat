@echo off
REM Kubernetes deployment script for Siqiniq-001

setlocal

echo ========================================
echo Siqiniq-001 Kubernetes Deployment
echo ========================================
echo.

set ACTION=apply

:parse_args
if "%1"=="" goto done_args
if /i "%1"=="--delete" set ACTION=delete
shift
goto parse_args

:done_args

if "%ACTION%"=="delete" (
    echo Deleting Kubernetes resources...
    kubectl delete -f docker/k8s/
    goto end
)

echo Deploying to Kubernetes...
echo.

REM Apply manifests in order
echo Creating namespace...
kubectl apply -f docker/k8s/00-namespace.yaml

echo.
echo Deploying MariaDB...
kubectl apply -f docker/k8s/01-mariadb.yaml

echo.
echo Deploying Redis...
kubectl apply -f docker/k8s/02-redis.yaml

echo.
echo Waiting for database to be ready...
kubectl wait --for=condition=ready pod -l app=mariadb -n siqiniq-001 --timeout=120s

echo.
echo Deploying game server...
kubectl apply -f docker/k8s/03-siqiniq-server.yaml

echo.
echo ========================================
echo Deployment Complete!
echo ========================================
echo.

echo Checking pod status...
kubectl get pods -n siqiniq-001

echo.
echo To view logs:
echo   kubectl logs -f -l app=siqiniq-server -n siqiniq-001
echo.
echo To get service info:
echo   kubectl get svc -n siqiniq-001
echo.
echo To port-forward for local access:
echo   kubectl port-forward svc/siqiniq-server 8080:8080 -n siqiniq-001
echo.

:end
endlocal
