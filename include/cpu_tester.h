/**
 * @file cpu_tester.h
 * @brief CPU peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the CPU tester class that implements comprehensive
 * testing and monitoring of CPU functionality on the NXP FRDM-IMX93 development board
 * with i.MX 93 processor featuring dual ARM Cortex-A55 cores.
 */

#ifndef CPU_TESTER_H
#define CPU_TESTER_H

#include <string>
#include <vector>

#include "peripheral_tester.h"

namespace imx93_peripheral_test {

/**
 * @struct CPUInfo
 * @brief Structure containing CPU information.
 */
struct CPUInfo {
  std::string model_name;
  int         cores;
  std::string architecture;
  double      frequency_mhz;
  double      temperature_c;
};

/**
 * @class CPUTester
 * @brief Tester implementation for CPU peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of CPU functionality including performance
 * benchmarking, temperature monitoring, and core validation.
 */
class CPUTester : public PeripheralTester {
public:
  /**
   * @brief Constructs a CPU tester instance.
   */
  CPUTester();

  /**
   * @brief Performs short verification test of CPU functionality.
   *
   * Tests basic CPU operations including:
   * - Core count verification
   * - Basic computation tests
   * - CPU information retrieval
   *
   * @return TestReport with detailed results.
   */
  TestReport short_test() override;

  /**
   * @brief Performs extended monitoring of CPU performance.
   *
   * Monitors CPU over time for:
   * - Temperature stability
   * - Performance consistency
   * - Load distribution across cores
   *
   * @param duration Monitoring duration in seconds.
   * @return TestReport with monitoring results.
   */
  TestReport monitor_test(std::chrono::seconds duration) override;

  /**
   * @brief Returns the peripheral name.
   * @return "CPU" as the peripheral identifier.
   */
  std::string get_peripheral_name() const override {
    return "CPU";
  }

  /**
   * @brief Checks if CPU is available on the system.
   * @return true if CPU information can be accessed.
   */
  bool is_available() const override;

private:
  /**
   * @brief Retrieves CPU information from system files.
   * @return CPUInfo structure with system CPU details.
   */
  CPUInfo get_cpu_info();

  /**
   * @brief Performs basic CPU computation benchmark.
   * @return TestResult indicating success or failure.
   */
  TestResult benchmark_cpu();

  /**
   * @brief Tests CPU temperature monitoring.
   * @return TestResult indicating success or failure.
   */
  TestResult test_temperature();

  /**
   * @brief Monitors CPU temperature over time.
   * @param duration Monitoring duration.
   * @return TestResult indicating success or failure.
   */
  TestResult monitor_temperature(std::chrono::seconds duration);

  /**
   * @brief Tests multi-core functionality.
   * @return TestResult indicating success or failure.
   */
  TestResult test_multi_core();

  /**
   * @brief Gets the current CPU temperature.
   * @return Temperature in Celsius, or -1.0 if not available.
   */
  double get_cpu_temperature();

  CPUInfo cpu_info_;
  bool    cpu_available_;
};

}  // namespace imx93_peripheral_test

#endif  // CPU_TESTER_H