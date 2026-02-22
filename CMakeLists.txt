cmake_minimum_required(VERSION 3.14)
project(quantum VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimized release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")

# Sources
set(SOURCES
    src/main.cpp
    src/Token.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/Value.cpp
    src/Interpreter.cpp
)

add_executable(quantum ${SOURCES})

target_include_directories(quantum PRIVATE include)

# Install to /usr/local/bin so `quantum` works globally
install(TARGETS quantum DESTINATION /usr/local/bin)

message(STATUS "Quantum Language compiler configured")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
