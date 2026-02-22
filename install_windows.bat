@echo off
setlocal

REM Install Quantum so it can be run as 'quantum' from any PowerShell/Terminal

set "SCRIPT_DIR=%~dp0"
set "TARGET_DIR=%LOCALAPPDATA%\Programs\Quantum"
set "EXE_SRC=%SCRIPT_DIR%quantum.exe"
set "EXE_DST=%TARGET_DIR%\quantum.exe"
set "CMD_DST=%TARGET_DIR%\quantum.cmd"

echo Installing Quantum to: "%TARGET_DIR%"
if not exist "%TARGET_DIR%" (
  mkdir "%TARGET_DIR%" || (
    echo Failed to create "%TARGET_DIR%". Try running this script as Administrator.
    exit /b 1
  )
)

if not exist "%EXE_SRC%" (
  echo quantum.exe not found, attempting to build with MSYS2 MinGW...
  if exist "C:\msys64\mingw64\bin\g++.exe" (
    "C:\msys64\mingw64\bin\g++.exe" -std=c++17 -O2 -Wall -I"%SCRIPT_DIR%include" ^
      "%SCRIPT_DIR%src\main.cpp" "%SCRIPT_DIR%src\Token.cpp" "%SCRIPT_DIR%src\Lexer.cpp" "%SCRIPT_DIR%src\Parser.cpp" "%SCRIPT_DIR%src\Value.cpp" "%SCRIPT_DIR%src\Interpreter.cpp" ^
      -o "%EXE_SRC%"
    if errorlevel 1 (
      echo Build failed. Please ensure MSYS2 MinGW toolchain is installed.
      exit /b 1
    )
  ) else (
    echo MSYS2 MinGW g++ not found. Build quantum.exe first, then re-run this installer.
    exit /b 1
  )
)

copy /Y "%EXE_SRC%" "%EXE_DST%" >nul
if errorlevel 1 (
  echo Failed to copy quantum.exe to "%TARGET_DIR%".
  exit /b 1
)

set "WINAPPS=%LOCALAPPDATA%\Microsoft\WindowsApps"
if exist "%WINAPPS%" (
  > "%WINAPPS%\quantum.cmd" echo @echo off
  >> "%WINAPPS%\quantum.cmd" echo "%EXE_DST%" %%*
)

REM Copy MSYS2 MinGW runtime DLLs when available
for %%D in ("C:\msys64\mingw64\bin\libstdc++-6.dll" "C:\msys64\mingw64\bin\libgcc_s_seh-1.dll" "C:\msys64\mingw64\bin\libwinpthread-1.dll") do (
  if exist %%D copy /Y %%D "%TARGET_DIR%" >nul
)

REM Create a simple command shim
> "%CMD_DST%" echo @echo off
>> "%CMD_DST%" echo "%EXE_DST%" %%*

REM Add TARGET_DIR to user PATH if not present
echo %PATH% | findstr /I /C:"%TARGET_DIR%" >nul
if errorlevel 1 (
echo Adding "%TARGET_DIR%" to user PATH...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p=[Environment]::GetEnvironmentVariable('Path','User');" ^
  "if ($p -notmatch [regex]::Escape('%TARGET_DIR%')) {" ^
  "  [Environment]::SetEnvironmentVariable('Path', $p + ';%TARGET_DIR%', 'User');" ^
  "  Write-Host 'PATH updated. Open a NEW PowerShell/Terminal for changes to take effect.';" ^
  "} else { Write-Host 'PATH already contains target.' }"

echo Installation complete.
echo Usage: quantum path\to\file.sa
echo Example: quantum "%SCRIPT_DIR%examples\hello.sa"
exit /b 0
