@echo off
REM This batch file runs the Quantum Language interpreter using MSYS2
C:\msys64\usr\bin\bash.exe -c "cd '/c/Programming/Compiler' && ./quantum_bin %*"
