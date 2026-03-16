cmake_minimum_required(VERSION 3.16)
project(Quantum CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/O2 /W3)
else()
    add_compile_options(-O2 -Wall -Wextra -Wno-unused-parameter)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# ── Shared sources (Lexer, Parser, Value, TypeChecker, etc.) ──────────────────
file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Remove all entry-point files from the shared pool
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main\\.cpp$")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*main_vm\\.cpp$")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*quantum_main\\.cpp$")
list(FILTER ALL_SOURCES EXCLUDE REGEX ".*Interpreter\\.cpp$")

# ── qrun: bytecode VM interpreter ─────────────────────────────────────────────
# Usage:  qrun file.sa     (interpreted mode, REPL, debugging)
add_executable(qrun
    ${ALL_SOURCES}
    src/main.cpp
)
if(WIN32)
    target_link_libraries(qrun advapi32)
endif()

# ── quantum: native compiler ──────────────────────────────────────────────────
# Usage:  quantum file.sa  (compile to native executable)
add_executable(quantum
    ${ALL_SOURCES}
    src/quantum_main.cpp
)
if(WIN32)
    target_link_libraries(quantum advapi32)
endif()

message(STATUS "Quantum v2.0.0")
message(STATUS "  qrun    — Bytecode VM interpreter  (qrun file.sa)")
message(STATUS "  quantum — Native compiler           (quantum file.sa)")