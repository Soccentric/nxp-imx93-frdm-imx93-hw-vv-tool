/**
 * @file gpio_tester.h
 * @brief GPIO peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the GPIO tester class that implements comprehensive
 * testing and monitoring of GPIO functionality on the NXP FRDM-IMX93 development board.
 *
 * @version 1.0
 * @date 2025-11-17
 *
 * @details
 * The GPIOTester class provides testing capabilities for:
 * - General Purpose I/O pins (GPIO1-GPIO5 banks)
 * - I²C interfaces (LPI2C1-LPI2C8)
 * - SPI interfaces (LPSPI1-LPSPI8)
 * - UART interfaces (LPUART1-LPUART8)
 * - PWM channels (TPM/FlexPWM)
 * - FlexIO configurable I/O
 *
 * The implementation uses the Linux GPIO sysfs interface and i.MX93
 * device tree overlays for hardware access.
 */

#ifndef GPIO_TESTER_H
#define GPIO_TESTER_H

#include "peripheral_tester.h"
#include <vector>
#include <string>

namespace imx93_peripheral_test {

/**
 * @enum GPIOMode
 * @brief GPIO pin modes for testing.
 */
enum class GPIOMode {
    INPUT,
    OUTPUT,
    PWM,
    I2C,
    SPI,
    UART
};

/**
 * @struct GPIOPin
 * @brief Structure representing a GPIO pin configuration.
 */
struct GPIOPin {
    int number;              /**< GPIO pin number */
    GPIOMode mode;           /**< Pin mode */
    bool pull_up;            /**< Pull-up resistor enabled */
    bool pull_down;          /**< Pull-down resistor enabled */
    int pwm_frequency;       /**< PWM frequency in Hz (if applicable) */
    int pwm_duty_cycle;      /**< PWM duty cycle percentage (if applicable) */
};

/**
 * @class GPIOTester
 * @brief Tester implementation for GPIO peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of GPIO functionality including digital I/O,
 * PWM generation, and communication interfaces.
 */
class GPIOTester : public PeripheralTester {
public:
    /**
     * @brief Constructs a GPIO tester instance.
     *
     * Initializes the GPIO tester with default pin configurations
     * for the FRDM-IMX93.
     */
    GPIOTester();

    /**
     * @brief Destructor that cleans up GPIO resources.
     */
    ~GPIOTester() override;

    /**
     * @brief Performs short verification test of GPIO functionality.
     *
     * Tests basic GPIO operations including:
     * - Pin export/unexport
     * - Direction setting (input/output)
     * - Digital read/write operations
     * - PWM basic functionality
     *
     * @return TestReport with detailed results.
     */
    TestReport short_test() override;

    /**
     * @brief Performs extended monitoring of GPIO peripherals.
     *
     * Monitors GPIO pins over time for:
     * - Signal stability
     * - PWM accuracy
     * - Interrupt handling
     * - Communication interface reliability
     *
     * @param duration Monitoring duration in seconds.
     * @return TestReport with monitoring results.
     */
    TestReport monitor_test(std::chrono::seconds duration) override;

    /**
     * @brief Returns the peripheral name.
     * @return "GPIO" as the peripheral identifier.
     */
    std::string get_peripheral_name() const override { return "GPIO"; }

    /**
     * @brief Checks if GPIO is available on the system.
     * @return true if GPIO sysfs interface is accessible.
     */
    bool is_available() const override;

private:
    /**
     * @brief Tests basic digital I/O operations.
     * @return TestResult indicating success or failure.
     */
    TestResult test_digital_io();

    /**
     * @brief Tests PWM functionality on available PWM pins.
     * @return TestResult indicating success or failure.
     */
    TestResult test_pwm();

    /**
     * @brief Tests I²C interfaces.
     * @return TestResult indicating success or failure.
     */
    TestResult test_i2c();

    /**
     * @brief Tests SPI interfaces.
     * @return TestResult indicating success or failure.
     */
    TestResult test_spi();

    /**
     * @brief Tests UART interfaces.
     * @return TestResult indicating success or failure.
     */
    TestResult test_uart();

    /**
     * @brief Monitors GPIO pins for stability over time.
     * @param duration Monitoring duration.
     * @return TestResult indicating success or failure.
     */
    TestResult monitor_gpio_stability(std::chrono::seconds duration);

    /**
     * @brief Exports a GPIO pin for use.
     * @param pin GPIO pin number.
     * @return true if export successful.
     */
    bool export_gpio(int pin);

    /**
     * @brief Unexports a GPIO pin.
     * @param pin GPIO pin number.
     * @return true if unexport successful.
     */
    bool unexport_gpio(int pin);

    /**
     * @brief Sets GPIO pin direction.
     * @param pin GPIO pin number.
     * @param output true for output, false for input.
     * @return true if direction set successfully.
     */
    bool set_gpio_direction(int pin, bool output);

    /**
     * @brief Reads GPIO pin value.
     * @param pin GPIO pin number.
     * @return pin value (0 or 1) or -1 on error.
     */
    int read_gpio(int pin);

    /**
     * @brief Writes GPIO pin value.
     * @param pin GPIO pin number.
     * @param value Value to write (0 or 1).
     * @return true if write successful.
     */
    bool write_gpio(int pin, int value);

    std::vector<GPIOPin> test_pins_; /**< List of pins to test */
    bool gpio_available_;            /**< GPIO availability flag */
};

} // namespace imx93_peripheral_test

#endif // GPIO_TESTER_H