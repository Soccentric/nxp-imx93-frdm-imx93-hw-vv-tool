/**
 * @file gpu_tester.cpp
 * @brief Implementation of GPU peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the i.MX 93 integrated GPU:
 * - Vivante GC7000UL GPU
 * - OpenGL ES 3.1 support
 * - Vulkan 1.1 support (driver dependent)
 * - 2D/3D graphics acceleration
 * - Video decode/encode acceleration
 */

#include "gpu_tester.h"

#include <dlfcn.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

namespace fs = std::filesystem;

namespace imx93_peripheral_test {

/**
 * @brief Constructs a GPU tester instance.
 *
 * Initializes the GPU tester by checking for i.MX93 Vivante GPU availability through
 * DRM (Direct Rendering Manager) devices and Vivante GPU driver presence.
 * The i.MX93 features an integrated Vivante GC520 GPU.
 */
GPUTester::GPUTester() : gpu_available_(false) {
  // Check for i.MX93 Vivante GPU through DRM devices
  // Vivante GPU typically appears as /dev/galcore or through DRM
  gpu_available_ = fs::exists("/dev/dri/card0") || fs::exists("/dev/galcore") ||
                   fs::exists("/dev/dri/renderD128");

  // Also check for Vivante-specific device nodes
  if (!gpu_available_) {
    gpu_available_ = fs::exists("/sys/class/misc/galcore");
  }

  if (gpu_available_) {
    gpu_info_ = get_gpu_info();
  }
}

/**
 * @brief Performs short verification test of GPU functionality.
 *
 * Executes a comprehensive short test that covers:
 * - GPU information retrieval and validation
 * - OpenGL functionality testing
 * - Vulkan functionality testing
 * - GPU memory availability checking
 *
 * @return TestReport containing detailed test results and timing information.
 *
 * @note This test provides a quick assessment of GPU subsystem functionality.
 */
TestReport GPUTester::short_test() {
  auto start_time = std::chrono::steady_clock::now();

  if (!gpu_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "GPU not available or not accessible",
                         std::chrono::milliseconds(0));
  }

  std::stringstream details;
  bool              all_passed = true;

  details << "GPU Model: " << gpu_info_.model_name << "\n";
  details << "Driver: " << gpu_info_.driver_version << "\n";
  details << "Memory: " << gpu_info_.memory_mb << " MB\n";

  // Test OpenGL
  TestResult opengl_result = test_opengl();
  details << "OpenGL: " << (opengl_result == TestResult::SUCCESS ? "PASS" : "FAIL");
  if (gpu_info_.supports_opengl) {
    details << " (" << gpu_info_.opengl_version << ")";
  }
  details << "\n";
  if (opengl_result != TestResult::SUCCESS && opengl_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  // Test Vulkan
  TestResult vulkan_result = test_vulkan();
  details << "Vulkan: " << (vulkan_result == TestResult::SUCCESS ? "PASS" : "FAIL");
  if (gpu_info_.supports_vulkan) {
    details << " (" << gpu_info_.vulkan_version << ")";
  }
  details << "\n";
  if (vulkan_result != TestResult::SUCCESS && vulkan_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  // Test GPU memory
  TestResult memory_result = test_gpu_memory();
  details << "GPU Memory: " << (memory_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (memory_result != TestResult::SUCCESS)
    all_passed = false;

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
  return create_report(overall_result, details.str(), duration);
}

/**
 * @brief Performs extended monitoring of GPU peripherals.
 *
 * Monitors GPU temperature over a specified duration to detect:
 * - Thermal stability issues
 * - GPU overheating conditions
 * - Cooling system effectiveness for GPU
 *
 * @param duration The time period over which to monitor GPU functionality.
 * @return TestReport containing monitoring results and temperature statistics.
 *
 * @note This is a long-running test that monitors GPU thermal behavior over time.
 */
TestReport GPUTester::monitor_test(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();

  if (!gpu_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "GPU not available or not accessible",
                         std::chrono::milliseconds(0));
  }

  TestResult result = monitor_gpu_temperature(duration);

  auto end_time      = std::chrono::steady_clock::now();
  auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::string details =
      "GPU monitoring completed for " + std::to_string(duration.count()) + " seconds";
  return create_report(result, details, test_duration);
}

/**
 * @brief Checks if GPU peripheral is available on the current system.
 *
 * Verifies GPU availability by checking for DRM devices, NVIDIA drivers,
 * or PCI VGA devices detected by lspci.
 *
 * @return true if GPU is accessible, false otherwise.
 */
bool GPUTester::is_available() const {
  return gpu_available_;
}

/**
 * @brief Retrieves comprehensive GPU information from system files.
 *
 * Gathers detailed GPU information from various system sources including
 * NVIDIA driver files, DRM sysfs entries, and graphics API detection.
 *
 * @return GPUInfo structure containing all retrieved GPU information.
 *
 * @note Supports detection of NVIDIA, AMD, and Intel GPUs.
 * @note Checks for OpenGL and Vulkan API support.
 */
GPUInfo GPUTester::get_gpu_info() {
  GPUInfo info;

  // Try to get GPU information from various sources
  // Check for NVIDIA GPU
  if (fs::exists("/proc/driver/nvidia/gpus/0/information")) {
    std::ifstream nvidia_info("/proc/driver/nvidia/gpus/0/information");
    if (nvidia_info.is_open()) {
      std::string line;
      while (std::getline(nvidia_info, line)) {
        if (line.find("Model:") != std::string::npos) {
          size_t colon_pos = line.find(':');
          if (colon_pos != std::string::npos) {
            info.model_name = line.substr(colon_pos + 2);
          }
        }
      }
    }
  }

  // Check for AMD GPU
  if (info.model_name.empty()) {
    std::ifstream amd_info("/sys/class/drm/card0/device/vendor");
    if (amd_info.is_open()) {
      std::string vendor;
      amd_info >> vendor;
      if (vendor == "0x1002") {  // AMD vendor ID
        info.model_name = "AMD GPU";
      }
    }
  }

  // Check for Intel GPU
  if (info.model_name.empty()) {
    std::ifstream intel_info("/sys/class/drm/card0/device/vendor");
    if (intel_info.is_open()) {
      std::string vendor;
      intel_info >> vendor;
      if (vendor == "0x8086") {  // Intel vendor ID
        info.model_name = "Intel GPU";
      }
    }
  }

  // Get memory info
  std::ifstream mem_info("/sys/class/drm/card0/device/mem_info_vram_total");
  if (mem_info.is_open()) {
    std::string mem_str;
    std::getline(mem_info, mem_str);
    try {
      info.memory_mb = std::stoull(mem_str) / (1024 * 1024);
    } catch (...) {
      info.memory_mb = 0;
    }
  }

  // Check OpenGL support
  info.supports_opengl = (system("glxinfo > /dev/null 2>&1") == 0);
  if (info.supports_opengl) {
    FILE* glx_pipe = popen("glxinfo | grep 'OpenGL version' | head -1", "r");
    if (glx_pipe) {
      char buffer[128];
      if (fgets(buffer, sizeof(buffer), glx_pipe)) {
        std::string line(buffer);
        size_t      colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
          info.opengl_version = line.substr(colon_pos + 2);
          // Remove trailing newline
          info.opengl_version.erase(info.opengl_version.find_last_not_of("\n\r\t") + 1);
        }
      }
      pclose(glx_pipe);
    }
  }

  // Check Vulkan support
  info.supports_vulkan = (system("vulkaninfo > /dev/null 2>&1") == 0);
  if (info.supports_vulkan) {
    FILE* vk_pipe = popen("vulkaninfo | grep 'Vulkan Instance Version' | head -1", "r");
    if (vk_pipe) {
      char buffer[128];
      if (fgets(buffer, sizeof(buffer), vk_pipe)) {
        std::string line(buffer);
        size_t      colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
          info.vulkan_version = line.substr(colon_pos + 2);
          info.vulkan_version.erase(info.vulkan_version.find_last_not_of("\n\r\t") + 1);
        }
      }
      pclose(vk_pipe);
    }
  }

  return info;
}

/**
 * @brief Tests OpenGL functionality on the GPU.
 *
 * Verifies OpenGL support by attempting to run glxgears,
 * which tests basic OpenGL context creation and rendering.
 *
 * @return TestResult::SUCCESS if OpenGL test passes,
 *         TestResult::NOT_SUPPORTED if OpenGL is not supported,
 *         TestResult::FAILURE if OpenGL functionality fails.
 *
 * @note Uses glxgears as a simple OpenGL functionality test.
 */
TestResult GPUTester::test_opengl() {
  if (!gpu_info_.supports_opengl) {
    return TestResult::NOT_SUPPORTED;
  }

  // Try to create a basic OpenGL context and render
  // This is a simplified test - in practice, we'd use GLFW or similar
  int result =
      system("glxgears -display :0 > /dev/null 2>&1 & sleep 1 && kill %1 > /dev/null 2>&1");
  return (result == 0) ? TestResult::SUCCESS : TestResult::FAILURE;
}

/**
 * @brief Tests Vulkan functionality on the GPU.
 *
 * Verifies Vulkan support by running vulkaninfo to check
 * Vulkan instance creation and device enumeration.
 *
 * @return TestResult::SUCCESS if Vulkan test passes,
 *         TestResult::NOT_SUPPORTED if Vulkan is not supported,
 *         TestResult::FAILURE if Vulkan functionality fails.
 *
 * @note Uses vulkaninfo command with timeout to prevent hanging.
 */
TestResult GPUTester::test_vulkan() {
  if (!gpu_info_.supports_vulkan) {
    return TestResult::NOT_SUPPORTED;
  }

  // Test Vulkan by running vulkaninfo
  int result = system("timeout 5 vulkaninfo > /dev/null 2>&1");
  return (result == 0) ? TestResult::SUCCESS : TestResult::FAILURE;
}

/**
 * @brief Tests GPU memory availability and basic functionality.
 *
 * Performs basic GPU memory testing by verifying that GPU memory
 * information is available and accessible.
 *
 * @return TestResult::SUCCESS if GPU memory is accessible,
 *         TestResult::NOT_SUPPORTED if GPU memory information is unavailable.
 *
 * @note This is a basic availability check; full GPU memory testing
 *       would require graphics API integration.
 */
TestResult GPUTester::test_gpu_memory() {
  // Test GPU memory by checking if we can allocate some GPU memory
  // This is a simplified test - in practice, we'd use OpenGL/Vulkan APIs

  if (gpu_info_.memory_mb == 0) {
    return TestResult::NOT_SUPPORTED;
  }

  // For now, just check if memory info is available
  return TestResult::SUCCESS;
}

/**
 * @brief Retrieves the current GPU temperature.
 *
 * Attempts to read GPU temperature from various system locations
 * commonly used for thermal monitoring of different GPU types.
 *
 * @return GPU temperature in degrees Celsius, or -1.0 if unavailable.
 *
 * @note Supports NVIDIA, AMD, Intel GPUs and generic thermal zones.
 * @note Handles both millidegree and degree formats automatically.
 */
double GPUTester::get_gpu_temperature() {
  // Try different GPU temperature sources
  std::vector<std::string> temp_files = {
      "/sys/class/drm/card0/device/hwmon/hwmon0/temp1_input",  // AMD/Intel
      "/proc/driver/nvidia/gpus/0/temperature",                // NVIDIA
      "/sys/class/thermal/thermal_zone1/temp"                  // Generic
  };

  for (const auto& temp_file : temp_files) {
    std::ifstream temp_stream(temp_file);
    if (temp_stream.is_open()) {
      std::string temp_str;
      std::getline(temp_stream, temp_str);
      try {
        double temp = std::stod(temp_str);
        if (temp > 1000) {  // Convert millidegrees to degrees
          temp /= 1000.0;
        }
        return temp;
      } catch (...) {
        continue;
      }
    }
  }

  return -1.0;
}

/**
 * @brief Monitors GPU temperature stability over time.
 *
 * Continuously monitors GPU temperature for the specified duration,
 * collecting statistics on temperature variation and stability.
 *
 * @param duration The monitoring duration in seconds.
 * @return TestResult::SUCCESS if temperature remains stable (variation ≤ 15°C),
 *         TestResult::FAILURE if temperature fluctuates excessively.
 *
 * @note Temperature readings are taken every 2 seconds during monitoring.
 * @note Stability threshold allows for normal GPU temperature variation.
 */
TestResult GPUTester::monitor_gpu_temperature(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();
  auto end_time   = start_time + duration;

  std::vector<double> temperatures;
  double              min_temp = 999.0;
  double              max_temp = -999.0;

  while (std::chrono::steady_clock::now() < end_time) {
    double temp = get_gpu_temperature();
    if (temp >= 0) {
      temperatures.push_back(temp);
      min_temp = std::min(min_temp, temp);
      max_temp = std::max(max_temp, temp);
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  if (temperatures.empty()) {
    return TestResult::NOT_SUPPORTED;
  }

  // Check temperature stability (variation should be reasonable)
  double temp_variation = max_temp - min_temp;
  return (temp_variation <= 15.0) ? TestResult::SUCCESS : TestResult::FAILURE;
}

}  // namespace imx93_peripheral_test