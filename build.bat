@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Quantum Language Builder
echo ========================================
echo.

set SCRIPT_DIR=%~dp0
set EXE=%SCRIPT_DIR%quantum.exe
set BUILD_DIR=%SCRIPT_DIR%build

:: Check if running as administrator for PATH modification
net session >nul 2>&1
if %errorlevel%==0 (
    echo [INFO] Running as administrator - can modify system PATH
    set IS_ADMIN=1
) else (
    echo [INFO] Running as user - will modify user PATH
    set IS_ADMIN=0
)

:: Clean previous build
echo [1/5] Cleaning previous build...
if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    echo     - Removed old build directory
)

:: Create build directory
echo [2/5] Creating build directory...
mkdir "%BUILD_DIR%"

:: Configure with CMake
echo [3/5] Configuring with CMake...
cmake -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

:: Build the project
echo [4/5] Building Quantum Language...
cmake --build "%BUILD_DIR%" --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

:: Copy executable to main directory
echo [5/5] Installing executable...
copy "%BUILD_DIR%\Release\quantum.exe" "%EXE%" >nul
if %errorlevel% neq 0 (
    echo [ERROR] Failed to copy executable!
    pause
    exit /b 1
)

:: Update quantum.bat to use new executable
echo [6/6] Updating launcher script...
(
echo @echo off
echo setlocal
echo set SCRIPT_DIR=%%~dp0
echo set EXE=%%SCRIPT_DIR%%quantum.exe
echo.
echo if exist "%%EXE%%" ^(
echo   "%%EXE%%" %%*
echo   exit /b %%ERRORLEVEL%%
echo ^)
echo.
echo echo quantum.exe was not found. Please run build.bat first.
echo exit /b 1
) > "%SCRIPT_DIR%quantum.bat"

:: Add to PATH
echo.
echo ========================================
echo PATH Configuration
echo ========================================

:: Check if already in PATH
echo %PATH% | findstr /i /c:"%SCRIPT_DIR%" >nul
if %errorlevel%==0 (
    echo [INFO] Quantum Language directory already in PATH
) else (
    echo [INFO] Adding Quantum Language to PATH...
    
    if %IS_ADMIN%==1 (
        :: System PATH
        setx PATH "%PATH%;%SCRIPT_DIR%" /M >nul 2>&1
        if !errorlevel!==0 (
            echo [SUCCESS] Added to system PATH
        ) else (
            echo [WARNING] Failed to modify system PATH
        )
    ) else (
        :: User PATH
        setx PATH "%PATH%;%SCRIPT_DIR%" >nul 2>&1
        if !errorlevel!==0 (
            echo [SUCCESS] Added to user PATH
        ) else (
            echo [WARNING] Failed to modify user PATH
        )
    )
    
    echo [NOTE] Restart your terminal to use PATH changes
)

:: Test the build
echo.
echo ========================================
echo Build Verification
echo ========================================
echo.
echo Testing quantum.exe:
"%EXE%" --version
if %errorlevel% neq 0 (
    echo [ERROR] Executable test failed!
    pause
    exit /b 1
)

echo.
echo Testing quantum.bat:
"%SCRIPT_DIR%quantum.bat" --version
if %errorlevel% neq 0 (
    echo [ERROR] Batch script test failed!
    pause
    exit /b 1
)

:: Test with example
echo.
echo Testing with example script:
"%EXE%" "%SCRIPT_DIR%examples\hello.sa"
if %errorlevel% neq 0 (
    echo [WARNING] Example test failed, but build succeeded
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Quantum Language has been successfully built and installed!
echo.
echo Usage:
echo   quantum --version          Show version
echo   quantum script.sa          Run a Quantum script
echo   quantum                    Start REPL
echo.
echo Executable location: %EXE%
echo Build directory: %BUILD_DIR%
echo.
if %IS_ADMIN%==0 (
    echo [NOTE] Restart PowerShell/CMD to use 'quantum' command directly
)
echo.
pause
