@echo off
setlocal
set SCRIPT_DIR=%~dp0
set EXE=%SCRIPT_DIR%quantum.exe

if exist "%EXE%" (
  "%EXE%" %*
  exit /b %ERRORLEVEL%
)

echo quantum.exe was not found. Please run build.bat first.
exit /b 1
