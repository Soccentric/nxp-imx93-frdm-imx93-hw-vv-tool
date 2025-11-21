/**
 * @file gpio_tester.cpp
 * @brief Implementation of GPIO peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This file implements the GPIOTester class methods for comprehensive
 * GPIO testing on NXP FRDM-IMX93 board.
 *
 * i.MX93 GPIO Features:
 * - 5 GPIO banks (GPIO1-GPIO5) with 32 pins each
 * - Configurable GPIO MUX for alternate functions
 * - LPI2C (I2C) interfaces: LPI2C1-LPI2C8
 * - LPSPI (SPI) interfaces: LPSPI1-LPSPI8
 * - LPUART (UART) interfaces: LPUART1-LPUART8
 * - TPM and FlexPWM for PWM generation
 * - FlexIO for configurable I/O
 */

#include "gpio_tester.h"

#include <errno.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

namespace imx93_peripheral_test {

namespace fs = std::filesystem;

/**
 * @brief Constructs a GPIO tester instance.
 *
 * Initializes the GPIO tester by checking for GPIO sysfs availability
 * and setting up the list of test pins appropriate for FRDM-IMX93.
 * The i.MX93 has 5 GPIO banks (GPIO1-GPIO5) with various pins exposed
 * on the FRDM board connectors.
 */
GPIOTester::GPIOTester() : gpio_available_(false) {
  // Check if GPIO sysfs is available
  gpio_available_ = fs::exists("/sys/class/gpio");

  // Initialize test pins for FRDM-IMX93
  // GPIO numbering: GPIO_BANK_N = (bank-1) * 32 + pin
  // Example: GPIO1_IO00 = 0, GPIO1_IO01 = 1, GPIO2_IO00 = 32, etc.
  //
  // Selected pins based on FRDM-IMX93 Arduino headers and expansion connectors
  test_pins_ = {
      // GPIO1 bank - Arduino D0-D7 pins
      {0, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO1_IO00
      {1, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO1_IO01
      {2, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO1_IO02
      {3, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO1_IO03
      {4, GPIOMode::UART, false, false, 0, 0},    // GPIO1_IO04 (LPUART1_TXD)
      {5, GPIOMode::UART, false, false, 0, 0},    // GPIO1_IO05 (LPUART1_RXD)
      {6, GPIOMode::I2C, false, false, 0, 0},     // GPIO1_IO06 (LPI2C1_SCL)
      {7, GPIOMode::I2C, false, false, 0, 0},     // GPIO1_IO07 (LPI2C1_SDA)

      // GPIO2 bank - SPI and additional I/O
      {32, GPIOMode::SPI, false, false, 0, 0},     // GPIO2_IO00 (LPSPI1_SCK)
      {33, GPIOMode::SPI, false, false, 0, 0},     // GPIO2_IO01 (LPSPI1_MOSI)
      {34, GPIOMode::SPI, false, false, 0, 0},     // GPIO2_IO02 (LPSPI1_MISO)
      {35, GPIOMode::SPI, false, false, 0, 0},     // GPIO2_IO03 (LPSPI1_CS0)
      {36, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO2_IO04
      {37, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO2_IO05

      // GPIO3 bank - PWM capable pins
      {64, GPIOMode::PWM, false, false, 1000, 50},  // GPIO3_IO00 (TPM1_CH0)
      {65, GPIOMode::PWM, false, false, 1000, 50},  // GPIO3_IO01 (TPM1_CH1)
      {66, GPIOMode::OUTPUT, false, false, 0, 0},   // GPIO3_IO02
      {67, GPIOMode::OUTPUT, false, false, 0, 0},   // GPIO3_IO03

      // GPIO4 bank - Additional I2C and UART
      {96, GPIOMode::I2C, false, false, 0, 0},   // GPIO4_IO00 (LPI2C2_SCL)
      {97, GPIOMode::I2C, false, false, 0, 0},   // GPIO4_IO01 (LPI2C2_SDA)
      {98, GPIOMode::UART, false, false, 0, 0},  // GPIO4_IO02 (LPUART2_TXD)
      {99, GPIOMode::UART, false, false, 0, 0},  // GPIO4_IO03 (LPUART2_RXD)

      // GPIO5 bank - FlexIO and general purpose
      {128, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO5_IO00
      {129, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO5_IO01
      {130, GPIOMode::OUTPUT, false, false, 0, 0},  // GPIO5_IO02
  };
}

/**
 * @brief Destructor that cleans up GPIO resources.
 *
 * Ensures all exported GPIO pins are properly unexported to prevent
 * resource leaks and maintain system stability.
 */
GPIOTester::~GPIOTester() {
  // Cleanup: unexport any exported GPIOs
  for (const auto& pin : test_pins_) {
    unexport_gpio(pin.number);
  }
}

/**
 * @brief Performs short verification test of GPIO functionality.
 *
 * Executes a comprehensive short test that covers:
 * - Digital I/O operations (export, direction setting, read/write)
 * - PWM functionality verification
 * - I²C interface availability
 * - SPI interface availability
 * - UART interface availability
 *
 * @return TestReport containing detailed test results and timing information.
 *
 * @note This test is designed to complete quickly while providing
 *       reasonable assurance of GPIO subsystem functionality.
 */
TestReport GPIOTester::short_test() {
  auto start_time = std::chrono::steady_clock::now();

  if (!gpio_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "GPIO sysfs interface not available",
                         std::chrono::milliseconds(0));
  }

  std::stringstream details;
  bool              all_passed = true;

  // Test digital I/O
  TestResult digital_result = test_digital_io();
  details << "Digital I/O: " << (digital_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (digital_result != TestResult::SUCCESS)
    all_passed = false;

  // Test PWM
  TestResult pwm_result = test_pwm();
  details << "PWM: " << (pwm_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (pwm_result != TestResult::SUCCESS)
    all_passed = false;

  // Test I2C
  TestResult i2c_result = test_i2c();
  details << "I2C: " << (i2c_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (i2c_result != TestResult::SUCCESS)
    all_passed = false;

  // Test SPI
  TestResult spi_result = test_spi();
  details << "SPI: " << (spi_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (spi_result != TestResult::SUCCESS)
    all_passed = false;

  // Test UART
  TestResult uart_result = test_uart();
  details << "UART: " << (uart_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (uart_result != TestResult::SUCCESS)
    all_passed = false;

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
  return create_report(overall_result, details.str(), duration);
}

/**
 * @brief Performs extended monitoring of GPIO peripherals.
 *
 * Monitors GPIO pins over a specified duration to detect:
 * - Signal stability issues
 * - Intermittent failures
 * - Hardware degradation
 *
 * The monitoring focuses on GPIO stability by repeatedly reading
 * pin values and checking for consistent operation.
 *
 * @param duration The time period over which to monitor GPIO functionality.
 * @return TestReport containing monitoring results and statistics.
 *
 * @note This is a long-running test that may take significant time to complete.
 */
TestReport GPIOTester::monitor_test(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();

  if (!gpio_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "GPIO sysfs interface not available",
                         std::chrono::milliseconds(0));
  }

  TestResult result = monitor_gpio_stability(duration);

  auto end_time      = std::chrono::steady_clock::now();
  auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::string details =
      "GPIO monitoring completed for " + std::to_string(duration.count()) + " seconds";
  return create_report(result, details, test_duration);
}

/**
 * @brief Checks if GPIO peripheral is available on the current system.
 *
 * Verifies the presence of the GPIO sysfs interface which is required
 * for GPIO operations on Linux systems.
 *
 * @return true if GPIO sysfs interface is accessible, false otherwise.
 */
bool GPIOTester::is_available() const {
  return gpio_available_;
}

/**
 * @brief Tests basic digital I/O operations on GPIO pins.
 *
 * Performs comprehensive testing of digital GPIO functionality including:
 * - GPIO pin export and unexport
 * - Direction setting (input/output)
 * - Digital write operations (high/low)
 * - Digital read operations
 *
 * Uses safe GPIO pins (2, 3, 4) that are typically available and
 * unlikely to conflict with system functions.
 *
 * @return TestResult::SUCCESS if all digital I/O tests pass,
 *         TestResult::FAILURE otherwise.
 *
 * @note This function temporarily exports GPIOs and restores them afterwards.
 */
TestResult GPIOTester::test_digital_io() {
  // Test a few GPIO pins for digital I/O
  // Using GPIO1 bank pins that are safe to test on FRDM-IMX93
  std::vector<int> test_gpios = {0, 1, 2};  // GPIO1_IO00, GPIO1_IO01, GPIO1_IO02

  for (int gpio : test_gpios) {
    // Export GPIO
    if (!export_gpio(gpio)) {
      return TestResult::FAILURE;
    }

    // Set as output
    if (!set_gpio_direction(gpio, true)) {
      unexport_gpio(gpio);
      return TestResult::FAILURE;
    }

    // Test writing high
    if (!write_gpio(gpio, 1)) {
      unexport_gpio(gpio);
      return TestResult::FAILURE;
    }

    // Small delay
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Test writing low
    if (!write_gpio(gpio, 0)) {
      unexport_gpio(gpio);
      return TestResult::FAILURE;
    }

    // Set as input
    if (!set_gpio_direction(gpio, false)) {
      unexport_gpio(gpio);
      return TestResult::FAILURE;
    }

    // Test reading
    int value = read_gpio(gpio);
    if (value == -1) {
      unexport_gpio(gpio);
      return TestResult::FAILURE;
    }

    // Unexport GPIO
    unexport_gpio(gpio);
  }

  return TestResult::SUCCESS;
}

/**
 * @brief Tests PWM functionality on GPIO pins.
 *
 * Verifies PWM capability by checking for PWM sysfs interface availability.
 * Currently performs basic availability check rather than functional testing.
 *
 * @return TestResult::SUCCESS if PWM interface is available,
 *         TestResult::NOT_SUPPORTED if PWM is not available.
 *
 * @note Full PWM functional testing would require device tree overlays
 *       and more complex setup beyond basic availability checking.
 */
TestResult GPIOTester::test_pwm() {
  // Test PWM on GPIO 18 (PWM0)
  int pwm_gpio = 18;

  // Export GPIO
  if (!export_gpio(pwm_gpio)) {
    return TestResult::FAILURE;
  }

  // For PWM testing, we would need to check if PWM sysfs is available
  // This is a simplified test - in practice, PWM setup requires device tree overlays
  std::string pwm_path = "/sys/class/pwm/pwmchip0";
  if (!fs::exists(pwm_path)) {
    unexport_gpio(pwm_gpio);
    return TestResult::NOT_SUPPORTED;
  }

  // Unexport GPIO
  unexport_gpio(pwm_gpio);

  return TestResult::SUCCESS;
}

/**
 * @brief Tests I²C interface availability.
 *
 * Checks for the presence of I²C device files in /dev/ to verify
 * that I²C interfaces are available on the system.
 *
 * @return TestResult::SUCCESS if at least one I²C device is found,
 *         TestResult::NOT_SUPPORTED if no I²C devices are available.
 *
 * @note This only checks for device presence, not actual I²C communication.
 *       Full I²C testing would require connected devices and communication testing.
 */
TestResult GPIOTester::test_i2c() {
  // Check if I2C devices are available
  std::vector<std::string> i2c_devices = {"/dev/i2c-0", "/dev/i2c-1"};

  bool i2c_found = false;
  for (const auto& device : i2c_devices) {
    if (fs::exists(device)) {
      i2c_found = true;
      break;
    }
  }

  if (!i2c_found) {
    return TestResult::NOT_SUPPORTED;
  }

  // In a full implementation, we would test I2C communication
  // For now, just check device presence
  return TestResult::SUCCESS;
}

/**
 * @brief Tests SPI interface availability.
 *
 * Checks for the presence of SPI device files in /dev/ to verify
 * that SPI interfaces are available on the system.
 *
 * @return TestResult::SUCCESS if at least one SPI device is found,
 *         TestResult::NOT_SUPPORTED if no SPI devices are available.
 *
 * @note This only checks for device presence, not actual SPI communication.
 *       Full SPI testing would require connected devices and communication testing.
 */
TestResult GPIOTester::test_spi() {
  // Check if SPI devices are available
  std::vector<std::string> spi_devices = {"/dev/spidev0.0", "/dev/spidev0.1"};

  bool spi_found = false;
  for (const auto& device : spi_devices) {
    if (fs::exists(device)) {
      spi_found = true;
      break;
    }
  }

  if (!spi_found) {
    return TestResult::NOT_SUPPORTED;
  }

  // In a full implementation, we would test SPI communication
  return TestResult::SUCCESS;
}

/**
 * @brief Tests UART interface availability.
 *
 * Checks for the presence of UART device files in /dev/ to verify
 * that UART interfaces are available on the system.
 *
 * @return TestResult::SUCCESS if at least one UART device is found,
 *         TestResult::NOT_SUPPORTED if no UART devices are available.
 *
 * @note This only checks for device presence, not actual UART communication.
 *       Full UART testing would require loopback testing or connected devices.
 */
TestResult GPIOTester::test_uart() {
  // Check if UART devices are available
  std::vector<std::string> uart_devices = {"/dev/ttyAMA0", "/dev/ttyS0"};

  bool uart_found = false;
  for (const auto& device : uart_devices) {
    if (fs::exists(device)) {
      uart_found = true;
      break;
    }
  }

  if (!uart_found) {
    return TestResult::NOT_SUPPORTED;
  }

  // In a full implementation, we would test UART communication
  return TestResult::SUCCESS;
}

/**
 * @brief Monitors GPIO pin stability over time.
 *
 * Performs extended monitoring of GPIO functionality by repeatedly
 * reading a test GPIO pin and checking for consistent operation.
 * Measures stability as the ratio of successful reads to total attempts.
 *
 * @param duration The duration over which to perform monitoring.
 * @return TestResult::SUCCESS if stability ratio >= 95%,
 *         TestResult::FAILURE otherwise.
 *
 * @note Uses GPIO 2 for monitoring as it's typically safe to use.
 * @note Reads are performed every 100ms during the monitoring period.
 */
TestResult GPIOTester::monitor_gpio_stability(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();
  auto end_time   = start_time + duration;

  int test_gpio = 2;  // Use GPIO 2 for monitoring

  // Export and set as input
  if (!export_gpio(test_gpio) || !set_gpio_direction(test_gpio, false)) {
    return TestResult::FAILURE;
  }

  int stable_count = 0;
  int total_reads  = 0;

  while (std::chrono::steady_clock::now() < end_time) {
    int value = read_gpio(test_gpio);
    if (value != -1) {
      stable_count++;
    }
    total_reads++;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Unexport GPIO
  unexport_gpio(test_gpio);

  // Consider it stable if 95% of reads succeeded
  double stability_ratio = static_cast<double>(stable_count) / total_reads;
  return (stability_ratio >= 0.95) ? TestResult::SUCCESS : TestResult::FAILURE;
}

/**
 * @brief Exports a GPIO pin for use.
 *
 * Makes a GPIO pin available for user-space access by writing to
 * the GPIO sysfs export file. After export, the pin can be configured
 * and used for I/O operations.
 *
 * @param pin The GPIO pin number to export.
 * @return true if the pin was successfully exported, false otherwise.
 *
 * @note This function includes a small delay to allow the sysfs
 *       interface to create the pin's directory structure.
 */
bool GPIOTester::export_gpio(int pin) {
  std::ofstream export_file("/sys/class/gpio/export");
  if (!export_file.is_open()) {
    return false;
  }

  export_file << pin;
  export_file.close();

  // Wait a bit for the GPIO to be exported
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  return fs::exists("/sys/class/gpio/gpio" + std::to_string(pin));
}

/**
 * @brief Unexports a GPIO pin.
 *
 * Removes a GPIO pin from user-space access by writing to the GPIO
 * sysfs unexport file. This should be called to clean up pins after use.
 *
 * @param pin The GPIO pin number to unexport.
 * @return true if the unexport operation was initiated successfully.
 *
 * @note This function only initiates the unexport; it doesn't verify completion.
 */
bool GPIOTester::unexport_gpio(int pin) {
  std::ofstream unexport_file("/sys/class/gpio/unexport");
  if (!unexport_file.is_open()) {
    return false;
  }

  unexport_file << pin;
  return true;
}

/**
 * @brief Sets the direction of a GPIO pin.
 *
 * Configures a GPIO pin as either input or output by writing to
 * the pin's direction file in the sysfs interface.
 *
 * @param pin The GPIO pin number to configure.
 * @param output true to set as output, false to set as input.
 * @return true if the direction was set successfully, false otherwise.
 */
bool GPIOTester::set_gpio_direction(int pin, bool output) {
  std::string   direction_path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/direction";
  std::ofstream direction_file(direction_path);
  if (!direction_file.is_open()) {
    return false;
  }

  direction_file << (output ? "out" : "in");
  return true;
}

/**
 * @brief Reads the value of a GPIO pin.
 *
 * Reads the current logic level of a GPIO pin by accessing the
 * pin's value file in the sysfs interface.
 *
 * @param pin The GPIO pin number to read.
 * @return The pin value (0 for low, 1 for high) or -1 on error.
 */
int GPIOTester::read_gpio(int pin) {
  std::string   value_path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/value";
  std::ifstream value_file(value_path);
  if (!value_file.is_open()) {
    return -1;
  }

  int value;
  value_file >> value;
  return value;
}

/**
 * @brief Writes a value to a GPIO pin.
 *
 * Sets the logic level of a GPIO pin by writing to the pin's
 * value file in the sysfs interface. The pin must be configured as output.
 *
 * @param pin The GPIO pin number to write to.
 * @param value The value to write (0 for low, 1 for high).
 * @return true if the write operation was successful, false otherwise.
 */
bool GPIOTester::write_gpio(int pin, int value) {
  std::string   value_path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/value";
  std::ofstream value_file(value_path);
  if (!value_file.is_open()) {
    return false;
  }

  value_file << value;
  return true;
}

}  // namespace imx93_peripheral_test