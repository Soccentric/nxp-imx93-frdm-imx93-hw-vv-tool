# Contributing to FRDM-IMX93 Peripheral Verification Tool

Thank you for your interest in contributing to the NXP FRDM-IMX93 Hardware Peripheral Verification Tool!

## Development Setup

### Prerequisites

- Linux system (Yocto Linux or Ubuntu recommended for full hardware access)
- CMake ≥ 3.20
- GCC ≥ 9.0 with C++17 support (ARM Cortex-A55 compatible)
- Clang tools (clang-format, clang-tidy)
- Doxygen (for documentation generation)
- Optional: NXP i.MX93 SDK and cross-compilation toolchain

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Soccentric/frdm-imx93-hardware-peripherals-verification-tool.git
cd frdm-imx93-hardware-peripherals-verification-tool

# Build
./scripts/build.sh

# Run tests
./scripts/test.sh
```

## Code Style

This project follows the Google C++ Style Guide with some modifications:

- **Indentation**: 2 spaces
- **Line Length**: 100 characters maximum
- **C++ Standard**: C++17
- **Naming Conventions**:
  - Classes: `PascalCase`
  - Functions/Methods: `snake_case`
  - Variables: `snake_case`
  - Constants: `UPPER_SNAKE_CASE`

### Formatting

Before submitting a pull request, format your code:

```bash
./scripts/format.sh
```

This will automatically format all C++ files using clang-format.

## Adding a New Peripheral

To add support for a new peripheral:

### 1. Create Header File

Create `include/new_peripheral_tester.h`:

```cpp
#ifndef NEW_PERIPHERAL_TESTER_H
#define NEW_PERIPHERAL_TESTER_H

#include "peripheral_tester.h"

namespace imx93_peripheral_test {

class NewPeripheralTester : public PeripheralTester {
public:
    NewPeripheralTester();
    ~NewPeripheralTester() override;
    
    TestReport short_test() override;
    TestReport monitor_test(std::chrono::seconds duration) override;
    std::string get_peripheral_name() const override;
    bool is_available() const override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace imx93_peripheral_test

#endif // NEW_PERIPHERAL_TESTER_H
```

### 2. Create Implementation

Create `libs/new_peripheral/new_peripheral_tester.cpp`:

```cpp
#include "new_peripheral_tester.h"
#include <iostream>

namespace imx93_peripheral_test {

class NewPeripheralTester::Impl {
public:
    // Implementation details
};

NewPeripheralTester::NewPeripheralTester() : pimpl_(std::make_unique<Impl>()) {}
NewPeripheralTester::~NewPeripheralTester() = default;

TestReport NewPeripheralTester::short_test() {
    auto start = std::chrono::steady_clock::now();
    
    // Implement test logic
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);
    
    return create_report(TestResult::SUCCESS, "Test details", duration);
}

TestReport NewPeripheralTester::monitor_test(std::chrono::seconds duration) {
    // Implement monitoring logic
}

std::string NewPeripheralTester::get_peripheral_name() const {
    return "NewPeripheral";
}

bool NewPeripheralTester::is_available() const {
    // Check if peripheral is available
    return true;
}

} // namespace cm5_peripheral_test
```

### 3. Create CMakeLists.txt

Create `libs/new_peripheral/CMakeLists.txt`:

```cmake
add_library(new_peripheral_tester STATIC)
target_sources(new_peripheral_tester
  PRIVATE
    new_peripheral_tester.cpp
)
target_include_directories(new_peripheral_tester
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(new_peripheral_tester PUBLIC cxx_std_17)

# Install
install(TARGETS new_peripheral_tester
  EXPORT cm5_peripheral_testTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
```

### 4. Update Build Configuration

Add to `libs/CMakeLists.txt`:
```cmake
add_subdirectory(new_peripheral)
```

### 5. Add to Main Application

Update `app/main.cpp` to include your peripheral in the test suite.

### 6. Write Unit Tests

Create tests in `tests/new_peripheral/test_new_peripheral_tester.cpp`:

```cpp
#include <gtest/gtest.h>
#include "new_peripheral_tester.h"

using namespace cm5_peripheral_test;

TEST(NewPeripheralTester, ShortTest) {
    NewPeripheralTester tester;
    if (tester.is_available()) {
        auto report = tester.short_test();
        EXPECT_NE(report.result, TestResult::FAILURE);
    }
}
```

### 7. Update Documentation

- Add peripheral to README.md supported peripherals table
- Document any special requirements or dependencies

## Pull Request Process

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes with clear messages
4. **Format** your code (`./scripts/format.sh`)
5. **Test** your changes (`./scripts/test.sh`)
6. **Push** to your branch (`git push origin feature/amazing-feature`)
7. **Open** a Pull Request

### Pull Request Guidelines

- Provide a clear description of the changes
- Reference any related issues
- Ensure all tests pass
- Update documentation as needed
- Follow the existing code style

## Testing

All new code should include appropriate tests:

```bash
# Run all tests
./scripts/test.sh

# Run specific test
cd build
./tests/new_peripheral/test_new_peripheral_tester
```

## Documentation

All public APIs must be documented using Doxygen-style comments:

```cpp
/**
 * @brief Brief description of the function.
 * 
 * Detailed description of what the function does,
 * its parameters, return value, and any side effects.
 *
 * @param param1 Description of parameter 1
 * @param param2 Description of parameter 2
 * @return Description of return value
 * @throws std::runtime_error if something goes wrong
 */
```

Generate documentation:
```bash
cd build
make doxygen
```

## Code Review

All submissions require review. We use GitHub pull requests for this purpose.

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

Feel free to open an issue for any questions or concerns!
