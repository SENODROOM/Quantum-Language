@echo off
setlocal

rem ── quantum.bat ───────────────────────────────────────────────────────────────
rem  Native compiler launcher — compiles .sa files to native executables.
rem
rem  Usage:  quantum file.sa
rem          quantum file.sa -o myapp
rem          quantum file.sa --run
rem          quantum file.sa --emit-c

set "QUANTUM_EXE="

if exist "%~dp0build\quantum.exe"          ( set "QUANTUM_EXE=%~dp0build\quantum.exe"          & goto :run )
if exist "%~dp0build\Release\quantum.exe"  ( set "QUANTUM_EXE=%~dp0build\Release\quantum.exe"  & goto :run )
if exist "%~dp0build\Debug\quantum.exe"    ( set "QUANTUM_EXE=%~dp0build\Debug\quantum.exe"    & goto :run )

echo [Error] quantum.exe not found. Run build.bat first.
exit /b 1

:run
"%QUANTUM_EXE%" %*
exit /b %errorlevel%