/**
 * @file power_tester.h
 * @brief Power peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the Power tester class that implements comprehensive
 * testing and monitoring of power management functionality on the NXP FRDM-IMX93.
 */

#ifndef POWER_TESTER_H
#define POWER_TESTER_H

#include "peripheral_tester.h"
#include <vector>
#include <string>
#include <cstdint>

namespace imx93_peripheral_test {

/**
 * @enum PowerSource
 * @brief Types of power sources.
 */
enum class PowerSource {
    BATTERY,
    AC_ADAPTER,
    USB_C,
    POE,
    UNKNOWN
};

/**
 * @enum PowerState
 * @brief Power management states.
 */
enum class PowerState {
    ACTIVE,
    SUSPEND,
    HIBERNATE,
    SHUTDOWN,
    UNKNOWN
};

/**
 * @struct PowerInfo
 * @brief Structure containing power information.
 */
struct PowerInfo {
    PowerSource source;
    PowerState state;
    double voltage_v;
    double current_ma;
    double power_w;
    int battery_percentage;
    bool battery_present;
    bool ac_connected;
    std::string power_supply_model;
};

/**
 * @struct PowerConsumption
 * @brief Structure containing power consumption data.
 */
struct PowerConsumption {
    double idle_power_w;
    double load_power_w;
    double suspend_power_w;
    double max_power_w;
};

/**
 * @class PowerTester
 * @brief Tester implementation for power peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of power management functionality including
 * power source detection, consumption monitoring, and battery management.
 */
class PowerTester : public PeripheralTester {
public:
    /**
     * @brief Constructs a Power tester instance.
     */
    PowerTester();

    /**
     * @brief Performs short verification test of power functionality.
     *
     * Tests basic power operations including:
     * - Power source detection
     * - Voltage/current monitoring
     * - Battery status (if present)
     *
     * @return TestReport with detailed results.
     */
    TestReport short_test() override;

    /**
     * @brief Performs extended monitoring of power consumption.
     *
     * Monitors power over time for:
     * - Power consumption stability
     * - Battery drain rate
     * - Power source switching
     *
     * @param duration Monitoring duration in seconds.
     * @return TestReport with monitoring results.
     */
    TestReport monitor_test(std::chrono::seconds duration) override;

    /**
     * @brief Returns the peripheral name.
     * @return "Power" as the peripheral identifier.
     */
    std::string get_peripheral_name() const override { return "Power"; }

    /**
     * @brief Checks if power management is available on the system.
     * @return true if power information can be accessed.
     */
    bool is_available() const override;

private:
    /**
     * @brief Retrieves power information from system.
     * @return PowerInfo structure with system power details.
     */
    PowerInfo get_power_info();

    /**
     * @brief Tests power source detection.
     * @return TestResult indicating success or failure.
     */
    TestResult test_power_source();

    /**
     * @brief Tests voltage and current monitoring.
     * @return TestResult indicating success or failure.
     */
    TestResult test_power_monitoring();

    /**
     * @brief Tests battery functionality if present.
     * @return TestResult indicating success or failure.
     */
    TestResult test_battery();

    /**
     * @brief Tests power management features.
     * @return TestResult indicating success or failure.
     */
    TestResult test_power_management();

    /**
     * @brief Monitors power consumption over time.
     * @param duration Monitoring duration.
     * @return TestResult indicating success or failure.
     */
    TestResult monitor_power_consumption(std::chrono::seconds duration);

    /**
     * @brief Measures power consumption under different loads.
     * @return PowerConsumption structure with measurements.
     */
    PowerConsumption measure_power_consumption();

    /**
     * @brief Parses power supply information from sysfs.
     * @param supply_path Path to power supply in sysfs.
     * @return PowerInfo structure.
     */
    PowerInfo parse_power_supply(const std::string& supply_path);

    PowerInfo power_info_;
    bool power_available_;
};

} // namespace imx93_peripheral_test

#endif // POWER_TESTER_H