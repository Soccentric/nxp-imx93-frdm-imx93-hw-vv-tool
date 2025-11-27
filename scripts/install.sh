#!/bin/bash
set -e

# FRDM-IMX93 Peripheral Verification Tool - Installation Script
echo "Installing FRDM-IMX93 Peripheral Verification Tool..."

# Install build dependencies
echo "Installing build dependencies..."
apt-get update
apt-get install -y qemu-user-static binfmt-support
update-binfmts --enable qemu-aarch64

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: Build directory not found. Please run ./scripts/build.sh first."
    exit 1
fi

# Check for root privileges
if [ "$EUID" -ne 0 ]; then 
    echo "This script requires root privileges. Please run with sudo."
    exit 1
fi

# Install
cd build
make install

echo ""
echo "Installation complete!"
echo ""
echo "The imx93_peripheral_test_app has been installed to /usr/local/bin/"
echo "You can now run: imx93_peripheral_test_app --help"
