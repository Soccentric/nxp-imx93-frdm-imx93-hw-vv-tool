#!/bin/bash
set -e

# FRDM-IMX93 Peripheral Verification Tool - Test Script
echo "Running tests..."

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: Build directory not found. Please run ./scripts/build.sh first."
    exit 1
fi

cd build
ctest --output-on-failure --verbose

echo "All tests completed!"