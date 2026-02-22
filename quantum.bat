@echo off
setlocal
set SCRIPT_DIR=%~dp0
set EXE=%SCRIPT_DIR%quantum.exe

if exist "%EXE%" (
  "%EXE%" %*
  exit /b %ERRORLEVEL%
)

echo quantum.exe was not found. Attempting to build...

rem Try MSYS2 MinGW g++ if available
if exist "C:\msys64\mingw64\bin\g++.exe" (
  "C:\msys64\mingw64\bin\g++.exe" -std=c++17 -O2 -Wall -I"%SCRIPT_DIR%include" ^
    "%SCRIPT_DIR%src\main.cpp" "%SCRIPT_DIR%src\Token.cpp" "%SCRIPT_DIR%src\Lexer.cpp" "%SCRIPT_DIR%src\Parser.cpp" "%SCRIPT_DIR%src\Value.cpp" "%SCRIPT_DIR%src\Interpreter.cpp" ^
    -o "%EXE%"
  if errorlevel 1 (
    echo Build failed with MinGW g++. Check your MSYS2 installation.
  ) else (
    "%EXE%" %*
    exit /b %ERRORLEVEL%
  )
)

echo Could not build automatically.
echo To build on Windows:
echo   Option A (MSYS2): install MSYS2 and run "mingw32-make" in this folder
echo   Option B (CMake): cmake -S . -B build && cmake --build build
echo   Option C (LLVM): install LLVM+MSVC Build Tools and use clang++
exit /b 1
