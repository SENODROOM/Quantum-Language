#!/bin/bash
set -e

BLUE='\033[34m'
GREEN='\033[32m'
RED='\033[31m'
YELLOW='\033[33m'
BOLD='\033[1m'
RESET='\033[0m'

echo ""
echo -e "${BLUE}${BOLD}  ╔══════════════════════════════════════╗${RESET}"
echo -e "${BLUE}${BOLD}  ║     Quantum Language Installer       ║${RESET}"
echo -e "${BLUE}${BOLD}  ╚══════════════════════════════════════╝${RESET}"
echo ""

# Check for cmake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}  ✗ cmake not found. Installing...${RESET}"
    sudo apt-get install -y cmake g++ > /dev/null 2>&1
fi

# Check for g++
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}  ✗ g++ not found. Installing...${RESET}"
    sudo apt-get install -y g++ > /dev/null 2>&1
fi

echo -e "${GREEN}  ✓ Dependencies found${RESET}"

# Build
echo -e "${YELLOW}  → Building...${RESET}"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local > /dev/null 2>&1
make -j$(nproc) 2>&1 | tail -3
echo -e "${GREEN}  ✓ Build successful${RESET}"

# Install
echo -e "${YELLOW}  → Installing to /usr/local/bin/quantum${RESET}"
sudo make install > /dev/null 2>&1
echo -e "${GREEN}  ✓ Installed!${RESET}"

echo ""
echo -e "${GREEN}${BOLD}  ╔══════════════════════════════════════╗${RESET}"
echo -e "${GREEN}${BOLD}  ║  Quantum is ready!                   ║${RESET}"
echo -e "${GREEN}${BOLD}  ║                                      ║${RESET}"
echo -e "${GREEN}${BOLD}  ║  Run:  quantum file.sa               ║${RESET}"
echo -e "${GREEN}${BOLD}  ║  REPL: quantum                       ║${RESET}"
echo -e "${GREEN}${BOLD}  ╚══════════════════════════════════════╝${RESET}"
echo ""
