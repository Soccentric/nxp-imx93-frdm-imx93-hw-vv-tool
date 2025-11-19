/**
 * @file display_tester.h
 * @brief Display peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the Display tester class that implements comprehensive
 * testing and monitoring of display functionality on the NXP FRDM-IMX93.
 */

#ifndef DISPLAY_TESTER_H
#define DISPLAY_TESTER_H

#include "peripheral_tester.h"
#include <vector>
#include <string>

namespace imx93_peripheral_test {

/**
 * @enum DisplayType
 * @brief Types of display interfaces.
 */
enum class DisplayType {
    HDMI,
    MIPI_DSI,
    COMPOSITE
};

/**
 * @struct DisplayInfo
 * @brief Structure containing display information.
 */
struct DisplayInfo {
    DisplayType type;
    std::string interface_name;
    std::string resolution;
    int refresh_rate;
    bool connected;
    bool enabled;
};

/**
 * @class DisplayTester
 * @brief Tester implementation for display peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of display functionality including HDMI and MIPI DSI
 * outputs on the NXP FRDM-IMX93.
 */
class DisplayTester : public PeripheralTester {
public:
    /**
     * @brief Constructs a Display tester instance.
     */
    DisplayTester();

    /**
     * @brief Performs short verification test of display functionality.
     *
     * Tests basic display operations including:
     * - Display detection and enumeration
     * - Resolution and refresh rate validation
     * - Basic output functionality
     *
     * @return TestReport with detailed results.
     */
    TestReport short_test() override;

    /**
     * @brief Performs extended monitoring of display stability.
     *
     * Monitors display over time for:
     * - Signal stability
     * - Resolution consistency
     * - Display connection status
     *
     * @param duration Monitoring duration in seconds.
     * @return TestReport with monitoring results.
     */
    TestReport monitor_test(std::chrono::seconds duration) override;

    /**
     * @brief Returns the peripheral name.
     * @return "Display" as the peripheral identifier.
     */
    std::string get_peripheral_name() const override { return "Display"; }

    /**
     * @brief Checks if display interfaces are available on the system.
     * @return true if display interfaces are accessible.
     */
    bool is_available() const override;

private:
    /**
     * @brief Enumerates all display interfaces on the system.
     * @return Vector of DisplayInfo structures.
     */
    std::vector<DisplayInfo> enumerate_displays();

    /**
     * @brief Tests HDMI display functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_hdmi();

    /**
     * @brief Tests MIPI DSI display functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_mipi_dsi();

    /**
     * @brief Tests display resolution and refresh rates.
     * @param display Display interface to test.
     * @return TestResult indicating success or failure.
     */
    TestResult test_display_resolution(const DisplayInfo& display);

    /**
     * @brief Tests display output by rendering a test pattern.
     * @return TestResult indicating success or failure.
     */
    TestResult test_display_output();

    /**
     * @brief Monitors display connection status over time.
     * @param duration Monitoring duration.
     * @return TestResult indicating success or failure.
     */
    TestResult monitor_display_connection(std::chrono::seconds duration);

    /**
     * @brief Tests 4Kp60 HDMI output capability.
     * @return TestResult indicating success or failure.
     */
    TestResult test_4k_hdmi();

    std::vector<DisplayInfo> displays_;
    bool display_available_;
};

} // namespace imx93_peripheral_test

#endif // DISPLAY_TESTER_H