#!/bin/bash
set -e

echo "=== AIDAW Build Script ==="
echo

mkdir -p build
cd build

echo "[1/2] Configuring CMake..."
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

echo "[2/2] Building..."
cmake --build . --config Release

echo
echo "=== Build complete! ==="
