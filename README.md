# FRDM-IMX93 Hardware Peripheral Verification Tool

## Overview
**FRDM-IMX93 Peripheral Verification Tool** is a comprehensive CMake-based C++ testing framework designed to verify and monitor all hardware peripherals on the NXP FRDM-IMX93 development board. The tool provides both short verification tests and extended monitoring capabilities for each peripheral, ensuring complete hardware validation.

## Purpose & Motivation
The NXP FRDM-IMX93 board features the i.MX 93 processor with dual Cortex-A55 cores and a wide array of hardware peripherals including CPU, GPU, memory, storage, display, camera, USB, networking, GPIO, and power management systems. Comprehensive testing of these peripherals is crucial for:

- Hardware validation during development
- Quality assurance in production
- Debugging hardware-related issues
- Performance monitoring and diagnostics
- Ensuring system stability over time

This tool addresses the need for systematic, automated testing of all CM5 peripherals with professional-grade C++ implementation and extensive documentation.

## Key Features
- **Modular Architecture** – Each peripheral has its own library and test suite
- **Dual Testing Modes** – Short verification tests and extended monitoring
- **Comprehensive Coverage** – Tests all major i.MX93 hardware peripherals
- **Professional Code Quality** – Full Doxygen documentation, modern C++17, PIMPL pattern
- **CMake Build System** – Cross-platform build configuration with testing support
- **Extensible Framework** – Easy to add new peripherals and test cases
- **Command-Line Interface** – Flexible CLI for individual or batch testing
- **NXP Optimized** – Tailored for i.MX93 architecture and peripherals

## Supported Peripherals

| Peripheral | Status | Short Test | Monitoring | Description |
|------------|--------|------------|------------|-------------|
| **CPU** | ✅ Implemented | Dual Cortex-A55 validation, benchmarking, temperature | Temperature stability | CPU performance and thermal monitoring |
| **GPIO** | ✅ Implemented | Digital I/O, PWM, I2C, SPI, UART, FlexIO | Signal stability | General-purpose I/O and communication interfaces |
| **GPU** | ✅ Implemented | Vivante GC520 OpenGL ES tests | Performance monitoring | Graphics processing unit validation |
| **Memory** | ✅ Implemented | DDR4/LPDDR4 testing, integrity validation | Memory integrity | System memory verification |
| **Storage** | ✅ Implemented | eMMC/SD/MMC tests | Wear monitoring | Storage device validation |
| **Display** | ✅ Implemented | MIPI-DSI, LVDS output tests | Display stability | Video output verification |
| **Camera** | ✅ Implemented | MIPI-CSI2 interface tests | Image capture validation | Camera module testing with ISI |
| **USB** | ✅ Implemented | USB 2.0 device enumeration, data transfer | Connection stability | USB host/device testing |
| **Networking** | ✅ Implemented | Dual Gigabit Ethernet tests | Bandwidth monitoring | ENET QoS controller validation |
| **Power** | ✅ Implemented | PMIC monitoring, voltage rails | Power stability | Power management testing |
| **Form Factor** | ✅ Implemented | Physical interface validation | Mechanical stability | FRDM board compliance testing |

## Architecture

### Core Components
- **PeripheralTester** – Abstract base class defining the testing interface
- **TestReport** – Structured test results with timing and diagnostic information
- **Modular Libraries** – Each peripheral implemented as a separate static library
- **Main Application** – CLI orchestrator for running tests across all peripherals

### Testing Framework
- **Short Tests** – Quick verification (seconds) of peripheral functionality
- **Monitoring Tests** – Extended testing (minutes/hours) for stability validation
- **Automated Reporting** – Detailed results with pass/fail status and diagnostics

## Getting Started

### Prerequisites
- Linux system (Yocto Linux or Ubuntu recommended for full hardware access)
- CMake ≥ 3.20
- GCC ≥ 9.0 with C++17 support (ARM Cortex-A55 compatible)
- Doxygen (for documentation generation)
- Google Test (automatically downloaded via FetchContent)
- NXP i.MX93 SDK (optional, for advanced features)

### Quick Start with Build Scripts
```bash
# Clone the repository
git clone https://github.com/Soccentric/frdm-imx93-hardware-peripherals-verification-tool.git
cd frdm-imx93-hardware-peripherals-verification-tool

# Build the project (creates optimized Release build)
./scripts/build.sh

# Run all tests
./scripts/test.sh

# Install system-wide (requires sudo)
sudo ./scripts/install.sh
```

### Manual Build & Install
```bash
# Create build directory
mkdir build && cd build

# Configure the project
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build all components
make -j$(nproc)

# Run tests
ctest --output-on-failure

# Generate documentation (optional)
make doxygen

# Install (requires sudo)
sudo make install
```

### Usage Examples

#### List Available Peripherals
```bash
imx93_peripheral_test_app --list
```

#### Run All Short Tests
```bash
imx93_peripheral_test_app --all-short
```

#### Run Individual Peripheral Tests
```bash
# CPU tests (dual Cortex-A55)
imx93_peripheral_test_app --test cpu
imx93_peripheral_test_app --monitor cpu 300

# GPIO tests (5 GPIO banks)
imx93_peripheral_test_app --test gpio
imx93_peripheral_test_app --monitor gpio 60

# Camera tests (MIPI-CSI2 with ISI)
imx93_peripheral_test_app --test camera
imx93_peripheral_test_app --monitor camera 60

# All peripherals support both short tests and monitoring
```

#### Run Monitoring Tests
```bash
# Monitor all peripherals for 10 minutes
imx93_peripheral_test_app --all-monitor 600
```

## Project Structure
```
frdm-imx93-hardware-peripherals-verification-tool/
├── .github/workflows/          # CI/CD automation
│   ├── ci.yml                 # Continuous integration
│   └── release.yml            # Automated releases
├── app/                        # Main application
│   ├── CMakeLists.txt
│   └── main.cpp               # CLI orchestrator
├── docs/                       # Documentation
│   ├── CMakeLists.txt
│   └── Doxyfile.in            # Doxygen configuration
├── include/                    # Public headers (12 files)
│   ├── peripheral_tester.h    # Base testing interface
│   ├── cpu_tester.h           # CPU peripheral (Cortex-A55)
│   ├── gpio_tester.h          # GPIO peripheral (5 banks)
│   ├── camera_tester.h        # Camera peripheral (MIPI-CSI2)
│   ├── gpu_tester.h           # GPU peripheral (Vivante GC520)
│   ├── memory_tester.h        # Memory peripheral (DDR4/LPDDR4)
│   ├── storage_tester.h       # Storage peripheral (eMMC/SD)
│   ├── display_tester.h       # Display peripheral (MIPI-DSI/LVDS)
│   ├── usb_tester.h           # USB peripheral (dual USB 2.0)
│   ├── networking_tester.h    # Networking peripheral (dual GbE)
│   ├── power_tester.h         # Power peripheral (PCA9451A PMIC)
│   └── form_factor_tester.h   # Form factor peripheral (FRDM board)
├── libs/                       # Peripheral libraries (11 modules)
│   ├── cpu/                   # CPU testing library
│   ├── gpio/                  # GPIO testing library
│   ├── camera/                # Camera testing library
│   ├── gpu/                   # GPU testing library
│   ├── memory/                # Memory testing library
│   ├── storage/               # Storage testing library
│   ├── display/               # Display testing library
│   ├── usb/                   # USB testing library
│   ├── networking/            # Networking testing library
│   ├── power/                 # Power testing library
│   └── form_factor/           # Form factor testing library
├── scripts/                    # Build and automation scripts
│   ├── build.sh               # Production build script
│   ├── test.sh                # Test execution script
│   ├── format.sh              # Code formatting script
│   └── install.sh             # Installation script
├── tests/                      # Unit tests (11 test suites)
│   ├── cpu/                   # CPU unit tests
│   ├── gpio/                  # GPIO unit tests
│   ├── camera/                # Camera unit tests
│   ├── gpu/                   # GPU unit tests
│   ├── memory/                # Memory unit tests
│   ├── storage/               # Storage unit tests
│   ├── display/               # Display unit tests
│   ├── usb/                   # USB unit tests
│   ├── networking/            # Networking unit tests
│   ├── power/                 # Power unit tests
│   └── form_factor/           # Form factor unit tests
├── .clang-format              # Code formatting rules
├── .clang-tidy                # Static analysis configuration
├── .editorconfig              # Editor configuration
├── .gitignore                 # Git ignore patterns
├── CHANGELOG.md               # Version history
├── CMakeLists.txt             # Main build configuration
├── CMakePresets.json          # CMake build presets
├── CONTRIBUTING.md            # Developer guidelines
├── LICENSE                    # MIT License
├── PRODUCTION_CHECKLIST.md    # Production readiness verification
└── README.md                  # This file
```

## Development Status

### Completed (v1.0.0)
- ✅ **Project Architecture** – Modular CMake-based build system
- ✅ **Base Framework** – PeripheralTester interface with PIMPL pattern
- ✅ **All 11 Peripheral Implementations**:
  - ✅ CPU (dual Cortex-A55, benchmarking, temperature, multi-core)
  - ✅ GPIO (5 banks, LPI2C, LPSPI, LPUART, FlexPWM, FlexIO)
  - ✅ GPU (Vivante GC520 OpenGL ES 3.1 validation)
  - ✅ Memory (DDR4/LPDDR4 integrity, ECC)
  - ✅ Storage (eMMC 5.1/SD/MMC via uSDHC)
  - ✅ Display (MIPI-DSI, LVDS output verification)
  - ✅ Camera (MIPI-CSI2 with ISI validation)
  - ✅ USB (dual USB 2.0 host/device functionality)
  - ✅ Networking (dual Gigabit Ethernet ENET QoS)
  - ✅ Power (PCA9451A PMIC monitoring, DVFS)
  - ✅ Form Factor (FRDM board Arduino/mikroBUS validation)
- ✅ **Complete Test Suite** – 11 GoogleTest-based test suites
- ✅ **Main Application** – Full CLI with all peripheral support
- ✅ **Documentation** – Doxygen, README, CONTRIBUTING, CHANGELOG
- ✅ **CI/CD** – GitHub Actions for testing and releases
- ✅ **Build Scripts** – Production-ready build, test, format, install scripts
- ✅ **Code Quality** – clang-format, clang-tidy, EditorConfig
- ✅ **Packaging** – DEB and TGZ package generation via CPack
- ✅ **i.MX93 Optimization** – ARM Cortex-A55 specific optimizations

### Production Ready ✅
All peripherals implemented, tested, and ready for production use on FRDM-IMX93!

## Contributing
Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

The project follows these development practices:

1. **Code Style** – Modern C++17 with RAII, PIMPL pattern, comprehensive error handling
2. **Documentation** – Complete Doxygen comments for all public APIs
3. **Testing** – Unit tests required for all new functionality (GoogleTest)
4. **Modularity** – Each peripheral is a separate library with its own tests
5. **Code Formatting** – Use `./scripts/format.sh` before committing
6. **CI/CD** – All PRs must pass automated tests

### Quick Development Workflow
```bash
# Format code
./scripts/format.sh

# Build
./scripts/build.sh

# Run tests
./scripts/test.sh

# Create a pull request
git checkout -b feature/my-feature
git add .
git commit -m "Add feature"
git push origin feature/my-feature
```

## API Reference

### PeripheralTester Interface
```cpp
class PeripheralTester {
public:
    virtual ~PeripheralTester() = default;
    virtual TestReport short_test() = 0;
    virtual TestReport monitor_test(std::chrono::seconds duration) = 0;
    virtual std::string get_peripheral_name() const = 0;
    virtual bool is_available() const = 0;
};
```

### Test Results
```cpp
enum class TestResult { SUCCESS, FAILURE, NOT_SUPPORTED, TIMEOUT, SKIPPED };

struct TestReport {
    TestResult result;
    std::string peripheral_name;
    std::chrono::milliseconds duration;
    std::string details;
    std::chrono::system_clock::time_point timestamp;
};
```

## License
This project is licensed under the **MIT License** – see the `LICENSE` file for details.

## Links & Resources
- **Repository**: [GitHub](https://github.com/Soccentric/frdm-imx93-hardware-peripherals-verification-tool)
- **Issues**: [GitHub Issues](https://github.com/Soccentric/frdm-imx93-hardware-peripherals-verification-tool/issues)
- **Documentation**: Build with `make doxygen` in build directory
- **Contributing**: See [CONTRIBUTING.md](CONTRIBUTING.md)
- **Changelog**: See [CHANGELOG.md](CHANGELOG.md)
- **NXP i.MX93**: [Product Page](https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/i-mx-applications-processors/i-mx-9-processors/i-mx-93-applications-processor-family:i.MX93)
- **FRDM-IMX93**: [Board Information](https://www.nxp.com/design/design-center/development-boards/i-mx-evaluation-and-development-boards/i-mx-93-evaluation-kit:FRDM-IMX93)

## Contact
- **Author**: Sandesh Ghimire
- **Organization**: Soccentric LLC
- **Email**: sandesh@soccentric.com

## Acknowledgments
Built with modern C++17, CMake, GoogleTest, and Doxygen. Designed specifically for the NXP FRDM-IMX93 development board with i.MX 93 applications processor.

---

**Status**: ✅ Production Ready v1.0.0 | **License**: MIT | **Platform**: NXP FRDM-IMX93 (i.MX 93)  
*Ensuring hardware reliability through comprehensive testing.*

