#!/bin/bash
set -e

# FRDM-IMX93 Peripheral Verification Tool - Format Script
echo "Formatting source code..."

# Format all C++ source and header files
find include app libs tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i

echo "Formatting complete!"