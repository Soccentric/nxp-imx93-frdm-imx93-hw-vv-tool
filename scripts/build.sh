#!/bin/bash
set -e

# FRDM-IMX93 Peripheral Verification Tool - Build Script
echo "Building FRDM-IMX93 Peripheral Verification Tool..."

# Configuration
BUILD_DIR="build"
BUILD_TYPE="${1:-Release}"
NUM_JOBS="$(nproc)"

# Create and configure build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure with CMake
echo "Configuring with BUILD_TYPE=${BUILD_TYPE}..."
echo "Target architecture: ARM Cortex-A55 (i.MX93)"
cmake .. \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_TESTING=ON

# Build
echo "Building with ${NUM_JOBS} parallel jobs..."
make -j"${NUM_JOBS}"

echo "Build complete! Binary: ${BUILD_DIR}/bin/imx93_peripheral_test_app"
echo "To run tests: cd ${BUILD_DIR} && ctest"
echo "To install: cd ${BUILD_DIR} && sudo make install"