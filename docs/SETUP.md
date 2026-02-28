# SETUP.md — Quantum Language Installation Guide

> **Quantum** v1.0.0 — A dynamically-typed, cybersecurity-ready scripting language written in C++17.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Build from Source](#build-from-source)
  - [Windows](#windows)
  - [Linux](#linux)
  - [macOS](#macos)
- [Adding Quantum to PATH](#adding-quantum-to-path)
- [Verifying Installation](#verifying-installation)
- [Running Your First Script](#running-your-first-script)
- [REPL Mode](#repl-mode)
- [Editor Support](#editor-support)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

Before building Quantum, ensure you have the following tools installed:

| Tool | Minimum Version | Notes |
|---|---|---|
| C++ Compiler | C++17 compatible | MSVC 2019+, GCC 9+, or Clang 10+ |
| CMake | 3.15+ | Required for all platforms |
| Git | Any recent version | For cloning the repository |

---

## Build from Source

### Clone the Repository

```bash
git clone https://github.com/SENODROOM/Quantum-Language.git
cd Quantum-Language
```

---

### Windows

#### Option A — CMake (Recommended)

```powershell
# Create and enter build directory
mkdir build
cd build

# Generate Visual Studio project or MinGW Makefiles
cmake ..

# Build in Release mode
cmake --build . --config Release
```

The binary will be at: `build\Release\quantum.exe`

#### Option B — build.bat (Quick Build)

A convenience batch script is included at the root:

```powershell
build.bat
```

This script handles the CMake configuration and build steps automatically on Windows.

#### Option C — MinGW / MSYS2

```bash
# In MSYS2 MinGW64 terminal
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
```

---

### Linux

```bash
# Install build tools if needed
sudo apt update
sudo apt install -y build-essential cmake git   # Ubuntu/Debian
# sudo dnf install gcc-c++ cmake git            # Fedora/RHEL

# Clone and build
git clone https://github.com/SENODROOM/Quantum-Language.git
cd Quantum-Language
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

The binary will be at: `build/quantum`

---

### macOS

```bash
# Install Xcode command-line tools if needed
xcode-select --install

# Install CMake via Homebrew if needed
brew install cmake

# Clone and build
git clone https://github.com/SENODROOM/Quantum-Language.git
cd Quantum-Language
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

The binary will be at: `build/quantum`

---

## Adding Quantum to PATH

To use `quantum` from any directory, add the binary to your system PATH.

### Windows (PowerShell)

```powershell
$env:PATH += ";C:\path\to\Quantum-Language\build\Release"
```

To make it permanent:

```powershell
[System.Environment]::SetEnvironmentVariable(
    "PATH",
    $env:PATH + ";C:\path\to\Quantum-Language\build\Release",
    [System.EnvironmentVariableTarget]::User
)
```

Alternatively, copy `quantum.exe` to a folder already in your PATH (e.g., `C:\Windows\System32` or a custom `C:\tools\bin`).

#### Using quantum.bat

A helper `quantum.bat` is included in the root directory. You can copy it anywhere in your PATH to invoke Quantum without needing the full path to the binary.

---

### Linux / macOS

```bash
# Copy to /usr/local/bin (system-wide)
sudo cp build/quantum /usr/local/bin/

# Or add the build directory to your shell profile
echo 'export PATH="$PATH:/path/to/Quantum-Language/build"' >> ~/.bashrc
source ~/.bashrc
```

---

## Verifying Installation

After setup, confirm everything works:

```bash
quantum --version
# → Quantum v1.0.0

quantum --help
# → Shows usage information
```

---

## Running Your First Script

1. Create a file called `hello.sa`:

```python
# hello.sa
name = "World"
print("Hello,", name, "from Quantum!")
```

2. Run it:

```bash
quantum hello.sa
# → Hello, World from Quantum!
```

3. Check syntax without executing:

```bash
quantum --check hello.sa
# → Syntax OK (or error details)
```

---

## REPL Mode

Start an interactive session by running `quantum` with no arguments:

```bash
quantum
```

```
Quantum v1.0.0 REPL
Type 'exit' or Ctrl+C to quit.

>>> x = 42
>>> print(x * 2)
84
>>> fn greet(name): return "Hello, " + name
>>> greet("Hacker")
"Hello, Hacker"
>>> exit
```

---

## Editor Support

Editor extensions for syntax highlighting are available in the `extensions/` directory. Currently supported:

| Editor | Status |
|---|---|
| VS Code | ✅ Available in `extensions/vscode/` |

### Installing the VS Code Extension

```bash
# From the repository root
cd extensions/vscode
npm install
npm run compile

# Then install via VS Code Extensions panel → "Install from VSIX"
# Or press F5 inside VS Code to launch a development instance
```

The extension provides:
- Syntax highlighting for `.sa` files
- Keyword recognition for all Quantum language constructs

---

## Troubleshooting

### "cmake: command not found"
Install CMake from [cmake.org/download](https://cmake.org/download/) or via your package manager.

### "No C++ compiler found"
- **Windows**: Install [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/) or MinGW via MSYS2
- **Linux**: Run `sudo apt install build-essential`
- **macOS**: Run `xcode-select --install`

### Build fails with C++17 error
Ensure your compiler supports C++17. Pass the standard explicitly:
```bash
cmake .. -DCMAKE_CXX_STANDARD=17
```

### `quantum` not found after PATH setup
Restart your terminal/shell session or run `source ~/.bashrc` on Linux/macOS.

### Permission denied on Linux/macOS
```bash
chmod +x build/quantum
```

---

## Summary

```
git clone https://github.com/SENODROOM/Quantum-Language.git
cd Quantum-Language && mkdir build && cd build
cmake .. && cmake --build . --config Release
sudo cp quantum /usr/local/bin/   # Linux/macOS
quantum --version                 # Verify
quantum script.sa                 # Run your script
```

---

*For full language reference, see [SYNTAX.md](SYNTAX.md).*