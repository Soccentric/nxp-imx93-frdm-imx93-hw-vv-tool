/**
 * @file storage_tester.h
 * @brief Storage peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the Storage tester class that implements comprehensive
 * testing and monitoring of storage functionality on the NXP FRDM-IMX93.
 */

#ifndef STORAGE_TESTER_H
#define STORAGE_TESTER_H

#include "peripheral_tester.h"
#include <vector>
#include <string>
#include <cstdint>

namespace imx93_peripheral_test {

/**
 * @enum StorageType
 * @brief Types of storage interfaces.
 */
enum class StorageType {
    EMMC,
    SDCARD,
    NVME,
    SATA,
    USB
};

/**
 * @struct StorageDevice
 * @brief Structure containing storage device information.
 */
struct StorageDevice {
    std::string device_path;
    StorageType type;
    std::string model;
    uint64_t size_gb;
    uint64_t available_gb;
    std::string filesystem;
    bool mounted;
};

/**
 * @class StorageTester
 * @brief Tester implementation for storage peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of storage functionality including eMMC, SD cards,
 * NVMe drives, and other storage interfaces.
 */
class StorageTester : public PeripheralTester {
public:
    /**
     * @brief Constructs a Storage tester instance.
     */
    StorageTester();

    /**
     * @brief Performs short verification test of storage functionality.
     *
     * Tests basic storage operations including:
     * - Device detection and enumeration
     * - Read/write operations
     * - Filesystem integrity
     *
     * @return TestReport with detailed results.
     */
    TestReport short_test() override;

    /**
     * @brief Performs extended monitoring of storage performance.
     *
     * Monitors storage over time for:
     * - I/O performance stability
     * - Wear monitoring (for flash storage)
     * - Filesystem health
     *
     * @param duration Monitoring duration in seconds.
     * @return TestReport with monitoring results.
     */
    TestReport monitor_test(std::chrono::seconds duration) override;

    /**
     * @brief Returns the peripheral name.
     * @return "Storage" as the peripheral identifier.
     */
    std::string get_peripheral_name() const override { return "Storage"; }

    /**
     * @brief Checks if storage devices are available on the system.
     * @return true if storage devices are accessible.
     */
    bool is_available() const override;

private:
    /**
     * @brief Enumerates all storage devices on the system.
     * @return Vector of StorageDevice structures.
     */
    std::vector<StorageDevice> enumerate_storage_devices();

    /**
     * @brief Tests eMMC storage functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_emmc();

    /**
     * @brief Tests SD card functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_sdcard();

    /**
     * @brief Tests NVMe storage functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_nvme();

    /**
     * @brief Tests PCIe storage functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_pcie();

    /**
     * @brief Tests M.2 socket functionality.
     * @return TestResult indicating success or failure.
     */
    TestResult test_m2();

    /**
     * @brief Performs read/write speed test on a storage device.
     * @param device_path Path to the storage device.
     * @return TestResult indicating success or failure.
     */
    TestResult test_storage_performance(const std::string& device_path);

    /**
     * @brief Monitors storage I/O over time.
     * @param duration Monitoring duration.
     * @return TestResult indicating success or failure.
     */
    TestResult monitor_storage_io(std::chrono::seconds duration);

    /**
     * @brief Tests filesystem integrity.
     * @param mount_point Filesystem mount point.
     * @return TestResult indicating success or failure.
     */
    TestResult test_filesystem_integrity(const std::string& mount_point);

    std::vector<StorageDevice> storage_devices_;
    bool storage_available_;
};

} // namespace imx93_peripheral_test

#endif // STORAGE_TESTER_H