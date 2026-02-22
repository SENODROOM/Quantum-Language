TARGET   = quantum
SRCS     = src/main.cpp src/Token.cpp src/Lexer.cpp src/Parser.cpp src/Value.cpp src/Interpreter.cpp
INCLUDES = -Iinclude

# Prefer MSYS2 MinGW g++ on Windows for a portable build
ifeq ($(OS),Windows_NT)
	CXX      = C:/msys64/mingw64/bin/g++.exe
else
	CXX      = g++
endif

CXXFLAGS = -std=c++17 -O2 -Wall $(INCLUDES)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRCS)
	@echo Building Quantum Language...
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)
	@echo Built: ./$(TARGET)

test: $(TARGET)
	@echo Running examples...
	./$(TARGET) examples/hello.sa
	./$(TARGET) examples/features.sa
	./$(TARGET) examples/cybersec.sa
	./$(TARGET) examples/advanced.sa
	@echo Done.

clean:
	@echo Cleaning...
	-@rm -f $(TARGET) src/*.o
	@echo Cleaned.
