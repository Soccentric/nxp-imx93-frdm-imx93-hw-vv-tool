/**
 * @file memory_tester.h
 * @brief Memory peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the Memory tester class that implements comprehensive
 * testing and monitoring of memory functionality on the NXP FRDM-IMX93.
 */

#ifndef MEMORY_TESTER_H
#define MEMORY_TESTER_H

#include "peripheral_tester.h"
#include <vector>
#include <string>
#include <cstdint>

namespace imx93_peripheral_test {

/**
 * @struct MemoryInfo
 * @brief Structure containing memory information.
 */
struct MemoryInfo {
    uint64_t total_ram_mb;
    uint64_t available_ram_mb;
    std::string memory_type;
    bool ecc_supported;
    bool ecc_enabled;
    uint32_t frequency_mhz;
};

/**
 * @class MemoryTester
 * @brief Tester implementation for memory peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of memory functionality including RAM integrity,
 * ECC validation, and performance monitoring.
 */
class MemoryTester : public PeripheralTester {
public:
    /**
     * @brief Constructs a Memory tester instance.
     */
    MemoryTester();

    /**
     * @brief Performs short verification test of memory functionality.
     *
     * Tests basic memory operations including:
     * - RAM availability and capacity
     * - Memory integrity testing
     * - ECC status (if supported)
     *
     * @return TestReport with detailed results.
     */
    TestReport short_test() override;

    /**
     * @brief Performs extended monitoring of memory performance.
     *
     * Monitors memory over time for:
     * - Memory usage stability
     * - Memory leak detection
     * - Bandwidth performance
     *
     * @param duration Monitoring duration in seconds.
     * @return TestReport with monitoring results.
     */
    TestReport monitor_test(std::chrono::seconds duration) override;

    /**
     * @brief Returns the peripheral name.
     * @return "Memory" as the peripheral identifier.
     */
    std::string get_peripheral_name() const override { return "Memory"; }

    /**
     * @brief Checks if memory is available on the system.
     * @return true if memory information can be accessed.
     */
    bool is_available() const override;

private:
    /**
     * @brief Retrieves memory information from system.
     * @return MemoryInfo structure with system memory details.
     */
    MemoryInfo get_memory_info();

    /**
     * @brief Tests RAM integrity with various patterns.
     * @return TestResult indicating success or failure.
     */
    TestResult test_ram_integrity();

    /**
     * @brief Tests memory bandwidth performance.
     * @return TestResult indicating success or failure.
     */
    TestResult test_memory_bandwidth();

    /**
     * @brief Tests ECC functionality if available.
     * @return TestResult indicating success or failure.
     */
    TestResult test_ecc();

    /**
     * @brief Monitors memory usage over time.
     * @param duration Monitoring duration.
     * @return TestResult indicating success or failure.
     */
    TestResult monitor_memory_usage(std::chrono::seconds duration);

    /**
     * @brief Performs memory stress test.
     * @param test_size_mb Size of memory to test in MB.
     * @return TestResult indicating success or failure.
     */
    TestResult stress_test_memory(size_t test_size_mb);

    MemoryInfo memory_info_;
    bool memory_available_;
};

} // namespace imx93_peripheral_test

#endif // MEMORY_TESTER_H