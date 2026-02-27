# Building Quantum Language

This guide explains how to build the Quantum Language compiler/interpreter from source code.

## Prerequisites

### Required Tools
- **C++ Compiler** with C++17 support:
  - GCC 7+ (Linux/macOS)
  - Clang 5+ (Linux/macOS)  
  - MSVC 2017+ (Windows)
- **CMake** 3.10 or higher
- **Make** or **Ninja** build system

### Platform-Specific Requirements

#### Windows
- Visual Studio 2017+ or Visual Studio Build Tools
- Git for Windows (if cloning from repository)
- CMake (can be installed via Visual Studio Installer or separately)

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential cmake git
```

#### macOS
```bash
xcode-select --install
brew install cmake git
```

## Build Methods

### Method 1: Using build.bat (Recommended - Windows)

#### Step 1: Run the build script
```cmd
build.bat
```

This script will:
- Clean previous builds
- Configure with CMake
- Build the project
- Install the executable
- Update PATH if needed
- Test the build

### Method 2: Using CMake (Manual)

#### Step 1: Configure Build
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### Step 2: Build
```bash
# Using make
make

# Or using ninja (if available)
ninja

# For parallel builds (use all CPU cores)
make -j$(nproc)  # Linux/macOS
make -j%NUMBER_OF_PROCESSORS%  # Windows
```

#### Step 3: Install (Optional)
```bash
make install
```

### Method 3: Using Make (Direct)

#### Step 1: Build with Makefile
```bash
make
```

#### Step 2: Clean Build
```bash
make clean
```

### Method 4: Using Visual Studio (Windows)

#### Step 1: Generate Visual Studio Solution
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"  # Adjust version as needed
```

#### Step 2: Build with Visual Studio
- Open `QuantumLanguage.sln` in Visual Studio
- Select `Release` or `Debug` configuration
- Build Solution (F7)

## Build Configurations

### Debug Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```
- Includes debug symbols
- No optimizations
- Useful for development and debugging

### Release Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
- Optimized for performance
- No debug symbols
- Recommended for distribution

### RelWithDebInfo Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make
```
- Optimized but includes debug symbols
- Good balance for profiling

## Build Outputs

After successful build, you'll find:

### Executable
- **Linux/macOS**: `quantum` (in build directory)
- **Windows**: `quantum.exe` (in build directory)

### Usage
```bash
# Run a Quantum script
./quantum examples/hello.sa

# Start interactive REPL
./quantum

# Check syntax only
./quantum --check script.sa

# Show version
./quantum --version
```

## Troubleshooting

### Common Issues

#### CMake Not Found
```bash
# Ubuntu/Debian
sudo apt install cmake

# macOS
brew install cmake

# Windows
# Download from https://cmake.org/download/
```

#### Compiler Not Found
```bash
# Ubuntu/Debian
sudo apt install build-essential

# CentOS/RHEL
sudo yum groupinstall "Development Tools"

# macOS
xcode-select --install
```

#### C++17 Support Issues
Ensure your compiler supports C++17:
```bash
# Check GCC version
gcc --version  # Should be 7.0 or higher

# Check Clang version
clang --version  # Should be 5.0 or higher
```

#### Build Fails on Windows
1. Ensure Visual Studio Build Tools are installed
2. Use Developer Command Prompt for VS
3. Check that CMake can find the compiler:
```cmd
cmake --version
cmake .. -G "Visual Studio 16 2019"
```

#### Permission Errors (Linux/macOS)
```bash
# Fix permissions if needed
chmod +x quantum
```

### Clean Build
If you encounter build issues, try a clean build:
```bash
# Remove build directory
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## Advanced Build Options

### Custom Installation Prefix
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make
sudo make install
```

### Using Ninja Build System
```bash
# Install ninja
# Ubuntu/Debian: sudo apt install ninja-build
# macOS: brew install ninja

cmake .. -G Ninja
ninja
```

### Static Build (Linux)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
make
```

## Development Build

For development with debugging symbols and warnings:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic"
make
```

## Cross-Platform Build Script

### build.sh (Linux/macOS)
```bash
#!/bin/bash
set -e

echo "Building Quantum Language..."

# Create build directory
mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

echo "Build complete! Executable: ./quantum"
```

### build.bat (Windows)
```batch
@echo off
echo Building Quantum Language...

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

echo Build complete! Executable: Release\quantum.exe
```

## Testing the Build

After building, verify the installation:

```bash
# Test version
./quantum --version

# Test with example script
./quantum ../examples/hello.sa

# Test REPL
echo 'print("Test")' | ./quantum
```

## Next Steps

After successful build:
1. Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand the compiler design
2. Check [examples/](../examples/) directory for sample Quantum scripts
3. Visit the [README.md](../README.md) for language documentation
4. Start developing your own Quantum applications!

## Build System Structure

```
Quantum Language/
├── CMakeLists.txt          # Main CMake configuration
├── build.bat               # Windows build script
├── src/                    # Source files
│   ├── main.cpp          # Entry point
│   ├── Lexer.cpp         # Lexical analyzer
│   ├── Parser.cpp        # Syntax parser
│   ├── Interpreter.cpp   # Runtime interpreter
│   └── ...
├── include/              # Header files
├── examples/             # Example Quantum scripts
└── build/               # Build output directory
```

For more detailed information about the compiler architecture and internals, see the [ARCHITECTURE.md](ARCHITECTURE.md) file.
