@echo off
setlocal
set IMAGE=regexhelper-builder
set SCRIPT_DIR=%~dp0
set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

docker build --platform linux/amd64 -f "%SCRIPT_DIR%\Dockerfile.linux" -t %IMAGE% "%SCRIPT_DIR%"
if errorlevel 1 ( echo [ERROR] docker build failed & exit /b 1 )

docker run --rm --platform linux/amd64 -v "%SCRIPT_DIR%:/src" %IMAGE% bash -c "sed -i 's/\r//g' /src/build_linux_inner.sh && bash /src/build_linux_inner.sh"
if errorlevel 1 ( echo [ERROR] build or verify failed & exit /b 1 )

echo Done: %SCRIPT_DIR%\build_linux\RegexHelper.so
