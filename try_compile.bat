@echo off
echo Trying different compilation methods...

echo Method 1: Direct g++ call
C:/msys64/ucrt64/bin/g++.exe -std=c++17 -Wall -Iinclude src/main.cpp src/Token.cpp src/Lexer.cpp src/Parser.cpp src/Value.cpp src/Interpreter.cpp -o quantum1.exe 2>method1_error.txt
if exist quantum1.exe echo Method 1 SUCCESS! && quantum1.exe examples\hello.sa && goto end

echo Method 2: Through bash
C:\msys64\usr\bin\bash.exe -c "cd /c/Programming/Compiler && g++ -std=c++17 -Wall -Iinclude src/main.cpp src/Token.cpp src/Lexer.cpp src/Parser.cpp src/Value.cpp src/Interpreter.cpp -o quantum2.exe" 2>method2_error.txt
if exist quantum2.exe echo Method 2 SUCCESS! && quantum2.exe examples\hello.sa && goto end

echo Method 3: Try clang
C:/Program Files/LLVM/bin/clang++.exe -std=c++17 -Wall -Iinclude src/main.cpp src/Token.cpp src/Lexer.cpp src/Parser.cpp src/Value.cpp src/Interpreter.cpp -o quantum3.exe 2>method3_error.txt
if exist quantum3.exe echo Method 3 SUCCESS! && quantum3.exe examples\hello.sa && goto end

echo All methods failed. Check error files:
type method1_error.txt
type method2_error.txt  
type method3_error.txt

:end
echo Done.
