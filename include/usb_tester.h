/**
 * @file usb_tester.h
 * @brief USB peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the USB tester class that implements comprehensive
 * testing and monitoring of USB functionality on the NXP FRDM-IMX93.
 */

#ifndef USB_TESTER_H
#define USB_TESTER_H

#include "peripheral_tester.h"
#include <vector>
#include <string>
#include <cstdint>

namespace imx93_peripheral_test {

/**
 * @enum USBVersion
 * @brief USB protocol versions.
 */
enum class USBVersion {
    USB1_0,
    USB1_1,
    USB2_0,
    USB3_0,
    USB3_1,
    USB3_2
};

/**
 * @enum USBDeviceType
 * @brief Types of USB devices.
 */
enum class USBDeviceType {
    HUB,
    STORAGE,
    INPUT_DEVICE,
    NETWORK_ADAPTER,
    AUDIO_DEVICE,
    VIDEO_DEVICE,
    OTHER
};

/**
 * @struct USBDeviceInfo
 * @brief Structure containing USB device information.
 */
struct USBDeviceInfo {
    std::string device_path;
    std::string vendor_id;
    std::string product_id;
    std::string manufacturer;
    std::string product_name;
    USBVersion version;
    USBDeviceType type;
    uint32_t max_power_ma;
    bool connected;
    bool high_speed;
    bool super_speed;
};

/**
 * @struct USBControllerInfo
 * @brief Structure containing USB controller information.
 */
struct USBControllerInfo {
    std::string controller_name;
    USBVersion max_version;
    uint32_t num_ports;
    bool ehci_available;
    bool ohci_available;
    bool xhci_available;
};

/**
 * @class USBTester
 * @brief Tester implementation for USB peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of USB functionality including device enumeration,
 * data transfer testing, and power management validation.
 */
class USBTester : public PeripheralTester {
public:
    /**
     * @brief Constructs a USB tester instance.
     */
    USBTester();

    /**
     * @brief Performs short verification test of USB functionality.
     *
     * Tests basic USB operations including:
     * - Controller availability
     * - Device enumeration
     * - Basic data transfer
     *
     * @return TestReport with detailed results.
     */
    TestReport short_test() override;

    /**
     * @brief Performs extended monitoring of USB performance.
     *
     * Monitors USB over time for:
     * - Device hotplug events
     * - Data transfer stability
     * - Power consumption
     *
     * @param duration Monitoring duration in seconds.
     * @return TestReport with monitoring results.
     */
    TestReport monitor_test(std::chrono::seconds duration) override;

    /**
     * @brief Returns the peripheral name.
     * @return "USB" as the peripheral identifier.
     */
    std::string get_peripheral_name() const override { return "USB"; }

    /**
     * @brief Checks if USB is available on the system.
     * @return true if USB controllers are accessible.
     */
    bool is_available() const override;

private:
    /**
     * @brief Retrieves USB controller information.
     * @return Vector of USBControllerInfo structures.
     */
    std::vector<USBControllerInfo> get_usb_controllers();

    /**
     * @brief Enumerates connected USB devices.
     * @return Vector of USBDeviceInfo structures.
     */
    std::vector<USBDeviceInfo> enumerate_usb_devices();

    /**
     * @brief Tests USB controller functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_usb_controllers();

    /**
     * @brief Tests USB device communication.
     * @param device The USB device to test.
     * @return TestResult indicating success or failure.
     */
    TestResult test_usb_device(const USBDeviceInfo& device);

    /**
     * @brief Tests USB data transfer performance.
     * @return TestResult indicating success or failure.
     */
    TestResult test_usb_transfer();

    /**
     * @brief Tests USB power management.
     * @return TestResult indicating success or failure.
     */
    TestResult test_usb_power();

    /**
     * @brief Monitors USB device connections over time.
     * @param duration Monitoring duration.
     * @return TestResult indicating success or failure.
     */
    TestResult monitor_usb_devices(std::chrono::seconds duration);

    /**
     * @brief Parses USB device information from sysfs.
     * @param device_path Path to the USB device in sysfs.
     * @return USBDeviceInfo structure.
     */
    USBDeviceInfo parse_usb_device_info(const std::string& device_path);

    std::vector<USBControllerInfo> controllers_;
    std::vector<USBDeviceInfo> devices_;
    bool usb_available_;
};

} // namespace imx93_peripheral_test

#endif // USB_TESTER_H