/**
 * @file form_factor_tester.h
 * @brief Physical form factor tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the Form Factor tester class that implements comprehensive
 * testing and verification of physical interfaces and form factor compliance
 * on the NXP FRDM-IMX93.
 */

#ifndef FORM_FACTOR_TESTER_H
#define FORM_FACTOR_TESTER_H

#include <cstdint>
#include <string>
#include <vector>

#include "peripheral_tester.h"

namespace imx93_peripheral_test {

/**
 * @enum InterfaceType
 * @brief Types of physical interfaces.
 */
enum class InterfaceType {
  GPIO,
  I2C,
  SPI,
  UART,
  PWM,
  I2S,
  PCM,
  HDMI,
  MIPI_DSI,
  MIPI_CSI,
  USB,
  ETHERNET,
  PCIe,
  SDIO,
  UNKNOWN
};

/**
 * @enum PinFunction
 * @brief GPIO pin functions.
 */
enum class PinFunction { INPUT, OUTPUT, ALT0, ALT1, ALT2, ALT3, ALT4, ALT5, UNKNOWN };

/**
 * @struct PinInfo
 * @brief Structure containing GPIO pin information.
 */
struct PinInfo {
  int         pin_number;
  PinFunction function;
  bool        pull_up;
  bool        pull_down;
  double      voltage_v;
  std::string description;
};

/**
 * @struct InterfaceInfo
 * @brief Structure containing interface information.
 */
struct InterfaceInfo {
  InterfaceType        type;
  std::string          name;
  bool                 available;
  std::string          status;
  std::vector<PinInfo> pins;
};

/**
 * @struct FormFactorInfo
 * @brief Structure containing form factor information.
 */
struct FormFactorInfo {
  std::string                module_type;
  std::string                revision;
  std::string                serial_number;
  double                     board_temperature_c;
  std::vector<InterfaceInfo> interfaces;
};

/**
 * @class FormFactorTester
 * @brief Tester implementation for physical form factor and interfaces.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of physical interfaces and form factor compliance
 * including GPIO pin testing, interface enumeration, and hardware validation.
 */
class FormFactorTester : public PeripheralTester {
public:
  /**
   * @brief Constructs a Form Factor tester instance.
   */
  FormFactorTester();

  /**
   * @brief Performs short verification test of physical interfaces.
   *
   * Tests basic interface operations including:
   * - GPIO pin functionality
   * - Interface enumeration
   * - Basic connectivity tests
   *
   * @return TestReport with detailed results.
   */
  TestReport short_test() override;

  /**
   * @brief Performs extended monitoring of physical interfaces.
   *
   * Monitors interfaces over time for:
   * - Signal integrity
   * - Temperature stability
   * - Interface reliability
   *
   * @param duration Monitoring duration in seconds.
   * @return TestReport with monitoring results.
   */
  TestReport monitor_test(std::chrono::seconds duration) override;

  /**
   * @brief Returns the peripheral name.
   * @return "Form Factor" as the peripheral identifier.
   */
  std::string get_peripheral_name() const override {
    return "Form Factor";
  }

  /**
   * @brief Checks if form factor testing is available on the system.
   * @return true if physical interfaces can be tested.
   */
  bool is_available() const override;

private:
  /**
   * @brief Retrieves form factor information from system.
   * @return FormFactorInfo structure with hardware details.
   */
  FormFactorInfo get_form_factor_info();

  /**
   * @brief Tests GPIO pin functionality.
   * @return TestResult indicating success or failure.
   */
  TestResult test_gpio_pins();

  /**
   * @brief Tests interface availability and basic functionality.
   * @return TestResult indicating success or failure.
   */
  TestResult test_interfaces();

  /**
   * @brief Tests board identification and revision.
   * @return TestResult indicating success or failure.
   */
  TestResult test_board_info();

  /**
   * @brief Tests temperature monitoring.
   * @return TestResult indicating success or failure.
   */
  TestResult test_temperature();

  /**
   * @brief Monitors interface stability over time.
   * @param duration Monitoring duration.
   * @return TestResult indicating success or failure.
   */
  TestResult monitor_interfaces(std::chrono::seconds duration);

  /**
   * @brief Enumerates available interfaces.
   * @return Vector of InterfaceInfo structures.
   */
  std::vector<InterfaceInfo> enumerate_interfaces();

  /**
   * @brief Tests specific GPIO pin.
   * @param pin_number GPIO pin number to test.
   * @return TestResult indicating success or failure.
   */
  TestResult test_gpio_pin(int pin_number);

  /**
   * @brief Gets board temperature.
   * @return Temperature in Celsius.
   */
  double get_board_temperature();

  FormFactorInfo form_factor_info_;
  bool           form_factor_available_;
};

}  // namespace imx93_peripheral_test

#endif  // FORM_FACTOR_TESTER_H