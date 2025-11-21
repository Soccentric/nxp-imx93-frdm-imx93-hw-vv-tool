/**
 * @file gpu_tester.h
 * @brief GPU peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the GPU tester class that implements comprehensive
 * testing and monitoring of GPU functionality on the NXP FRDM-IMX93.
 */

#ifndef GPU_TESTER_H
#define GPU_TESTER_H

#include <string>
#include <vector>

#include "peripheral_tester.h"

namespace imx93_peripheral_test {

/**
 * @struct GPUInfo
 * @brief Structure containing GPU information.
 */
struct GPUInfo {
  std::string model_name;
  std::string driver_version;
  std::string opengl_version;
  std::string vulkan_version;
  uint64_t    memory_mb;
  bool        supports_opengl;
  bool        supports_vulkan;
};

/**
 * @class GPUTester
 * @brief Tester implementation for GPU peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of GPU functionality including OpenGL and Vulkan
 * validation, memory testing, and performance monitoring.
 */
class GPUTester : public PeripheralTester {
public:
  /**
   * @brief Constructs a GPU tester instance.
   */
  GPUTester();

  /**
   * @brief Performs short verification test of GPU functionality.
   *
   * Tests basic GPU operations including:
   * - Driver availability
   * - OpenGL/Vulkan support
   * - Basic rendering capabilities
   *
   * @return TestReport with detailed results.
   */
  TestReport short_test() override;

  /**
   * @brief Performs extended monitoring of GPU performance.
   *
   * Monitors GPU over time for:
   * - Temperature stability
   * - Memory usage
   * - Rendering performance
   *
   * @param duration Monitoring duration in seconds.
   * @return TestReport with monitoring results.
   */
  TestReport monitor_test(std::chrono::seconds duration) override;

  /**
   * @brief Returns the peripheral name.
   * @return "GPU" as the peripheral identifier.
   */
  std::string get_peripheral_name() const override {
    return "GPU";
  }

  /**
   * @brief Checks if GPU is available on the system.
   * @return true if GPU drivers are accessible.
   */
  bool is_available() const override;

private:
  /**
   * @brief Retrieves GPU information from system.
   * @return GPUInfo structure with system GPU details.
   */
  GPUInfo get_gpu_info();

  /**
   * @brief Tests OpenGL functionality.
   * @return TestResult indicating success or failure.
   */
  TestResult test_opengl();

  /**
   * @brief Tests Vulkan functionality.
   * @return TestResult indicating success or failure.
   */
  TestResult test_vulkan();

  /**
   * @brief Tests GPU memory.
   * @return TestResult indicating success or failure.
   */
  TestResult test_gpu_memory();

  /**
   * @brief Gets GPU temperature.
   * @return Temperature in Celsius, or -1.0 if not available.
   */
  double get_gpu_temperature();

  /**
   * @brief Monitors GPU temperature over time.
   * @param duration Monitoring duration.
   * @return TestResult indicating success or failure.
   */
  TestResult monitor_gpu_temperature(std::chrono::seconds duration);

  GPUInfo gpu_info_;
  bool    gpu_available_;
};

}  // namespace imx93_peripheral_test

#endif  // GPU_TESTER_H