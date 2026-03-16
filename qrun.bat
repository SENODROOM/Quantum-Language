@echo off
setlocal

rem ── qrun.bat ──────────────────────────────────────────────────────────────────
rem  Interpreted mode launcher — runs .sa files on the Quantum Bytecode VM.
rem  Supports REPL, debugging, disassembly, and batch testing.
rem
rem  Usage:  qrun file.sa
rem          qrun                    (REPL)
rem          qrun --debug file.sa
rem          qrun --dis   file.sa
rem          qrun --test  examples/

set "QRUN_EXE="

if exist "%~dp0build\qrun.exe"          ( set "QRUN_EXE=%~dp0build\qrun.exe"          & goto :run )
if exist "%~dp0build\Release\qrun.exe"  ( set "QRUN_EXE=%~dp0build\Release\qrun.exe"  & goto :run )
if exist "%~dp0build\Debug\qrun.exe"    ( set "QRUN_EXE=%~dp0build\Debug\qrun.exe"    & goto :run )

echo [Error] qrun.exe not found. Run build.bat first.
exit /b 1

:run
"%QRUN_EXE%" %*
exit /b %errorlevel%