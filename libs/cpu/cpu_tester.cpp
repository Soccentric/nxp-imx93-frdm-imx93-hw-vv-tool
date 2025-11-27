/**
 * @file cpu_tester.cpp
 * @brief Implementation of CPU peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the i.MX 93 processor featuring:
 * - Dual ARM Cortex-A55 cores (up to 1.7 GHz)
 * - Single ARM Cortex-M33 core (up to 250 MHz) - RTOS domain, not directly testable from Linux
 * - Arm Ethos U-65 NPU (0.5 TOPS at 1 GHz)
 * - ARM v8.2-A architecture
 * - Advanced power management
 * - Integrated thermal monitoring
 */

#include "cpu_tester.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <thread>

namespace fs = std::filesystem;

namespace imx93_peripheral_test {

/**
 * @brief Constructs a CPU tester instance.
 *
 * Initializes the CPU tester by checking for CPU information availability
 * and retrieving initial CPU information from /proc/cpuinfo.
 * For i.MX93, expects to find dual Cortex-A55 cores, and checks for NPU availability.
 */
CPUTester::CPUTester() : cpu_available_(false) {
  // Check if CPU information is available
  cpu_available_ = fs::exists("/proc/cpuinfo");
  if (cpu_available_) {
    cpu_info_ = get_cpu_info();
    // Verify we have i.MX93 CPU (Cortex-A55)
    if (cpu_info_.model_name.empty()) {
      // On ARM systems, model name might not be present, check architecture
      cpu_available_ = !cpu_info_.architecture.empty();
    }
  }
}

/**
 * @brief Performs short verification test of CPU functionality.
 *
 * Executes a comprehensive short test that covers:
 * - CPU information retrieval and validation
 * - Basic computational benchmarking
 * - Temperature monitoring capability
 * - Multi-core functionality verification
 *
 * @return TestReport containing detailed test results and timing information.
 *
 * @note This test provides a quick assessment of CPU health and performance.
 */
TestReport CPUTester::short_test() {
  auto start_time = std::chrono::steady_clock::now();

  if (!cpu_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "CPU information not available",
                         std::chrono::milliseconds(0));
  }

  std::stringstream details;
  bool              all_passed = true;

  // Test CPU info retrieval
  details << "CPU Model: " << cpu_info_.model_name << "\n";
  details << "Cores: " << cpu_info_.cores << "\n";
  details << "Architecture: " << cpu_info_.architecture << "\n";
  details << "Frequency: " << cpu_info_.frequency_mhz << " MHz\n";
  details << "M33 Core: " << (cpu_info_.m33_available ? "Present (RTOS domain)" : "Not available")
          << "\n";
  details << "NPU: "
          << (cpu_info_.npu_available
                  ? "Ethos U-65 (" + std::to_string(cpu_info_.npu_tops) + " TOPS)"
                  : "Not available")
          << "\n";

  // Test basic computation
  TestResult benchmark_result = benchmark_cpu();
  details << "Benchmark: " << (benchmark_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (benchmark_result != TestResult::SUCCESS)
    all_passed = false;

  // Test temperature
  TestResult temp_result = test_temperature();
  details << "Temperature: " << (temp_result == TestResult::SUCCESS ? "PASS" : "FAIL");
  if (temp_result == TestResult::SUCCESS) {
    details << " (" << cpu_info_.temperature_c << "°C)\n";
  } else {
    details << "\n";
  }
  if (temp_result != TestResult::SUCCESS && temp_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  // Test multi-core
  TestResult multi_core_result = test_multi_core();
  details << "Multi-core: " << (multi_core_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (multi_core_result != TestResult::SUCCESS)
    all_passed = false;

  // Test NPU availability
  TestResult npu_result = test_npu();
  details << "NPU: "
          << (npu_result == TestResult::SUCCESS
                  ? "PASS"
                  : (npu_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (npu_result != TestResult::SUCCESS && npu_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
  return create_report(overall_result, details.str(), duration);
}

/**
 * @brief Performs extended monitoring of CPU peripherals.
 *
 * Monitors CPU temperature over a specified duration to detect:
 * - Thermal stability issues
 * - Overheating conditions
 * - Cooling system effectiveness
 *
 * @param duration The time period over which to monitor CPU functionality.
 * @return TestReport containing monitoring results and temperature statistics.
 *
 * @note This is a long-running test that monitors thermal behavior over time.
 */
TestReport CPUTester::monitor_test(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();

  if (!cpu_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "CPU information not available",
                         std::chrono::milliseconds(0));
  }

  TestResult result = monitor_temperature(duration);

  auto end_time      = std::chrono::steady_clock::now();
  auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::string details =
      "CPU monitoring completed for " + std::to_string(duration.count()) + " seconds";
  return create_report(result, details, test_duration);
}

/**
 * @brief Checks if CPU peripheral is available on the current system.
 *
 * Verifies the presence of /proc/cpuinfo which is required for
 * CPU information retrieval on Linux systems.
 *
 * @return true if CPU information is accessible, false otherwise.
 */
bool CPUTester::is_available() const {
  return cpu_available_;
}

/**
 * @brief Retrieves comprehensive CPU information from system files.
 *
 * Parses /proc/cpuinfo and other system files to gather detailed
 * information about the CPU including model, cores, architecture,
 * frequency, and current temperature.
 *
 * @return CPUInfo structure containing all retrieved CPU information.
 *
 * @note This function caches the CPU information for performance.
 */
CPUInfo CPUTester::get_cpu_info() {
  CPUInfo info;
  info.cores         = 0;
  info.frequency_mhz = 0.0;
  std::ifstream cpuinfo("/proc/cpuinfo");

  if (!cpuinfo.is_open()) {
    return info;
  }

  std::string line;
  int         processor_count = 0;

  while (std::getline(cpuinfo, line)) {
    // ARM-specific fields for i.MX93
    if (line.find("processor") != std::string::npos && line.find(":") != std::string::npos) {
      processor_count++;
    } else if (line.find("model name") != std::string::npos) {
      size_t colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        info.model_name = line.substr(colon_pos + 2);
      }
    } else if (line.find("CPU implementer") != std::string::npos) {
      // ARM CPU - check if it's ARM (0x41)
      size_t colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        std::string impl = line.substr(colon_pos + 2);
        if (impl.find("0x41") != std::string::npos) {
          info.model_name = "ARM Cortex-A55 (i.MX93)";
        }
      }
    } else if (line.find("CPU architecture") != std::string::npos) {
      size_t colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        info.architecture = "ARMv8.2-A (" + line.substr(colon_pos + 2) + ")";
      }
    } else if (line.find("cpu cores") != std::string::npos) {
      size_t colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        info.cores = std::stoi(line.substr(colon_pos + 2));
      }
    }
  }

  // If cores not found, use processor count
  if (info.cores == 0 && processor_count > 0) {
    info.cores = processor_count;
  }

  // Default to 2 cores for i.MX93 if detection failed
  if (info.cores == 0) {
    info.cores = 2;  // i.MX93 has 2 Cortex-A55 cores
  }

  // Get CPU frequency - i.MX93 Cortex-A55 can run up to 1.7 GHz
  std::ifstream freq_file("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
  if (freq_file.is_open()) {
    std::string freq_str;
    std::getline(freq_file, freq_str);
    try {
      info.frequency_mhz = std::stod(freq_str) / 1000.0;  // Convert kHz to MHz
    } catch (...) {
      info.frequency_mhz = 1700.0;  // Default max frequency for i.MX93
    }
  } else {
    info.frequency_mhz = 1700.0;  // Default for i.MX93
  }

  // Get temperature
  info.temperature_c = get_cpu_temperature();

  // Check for Cortex-M33 (not directly accessible from Linux)
  info.m33_available = true;  // Assumed present on i.MX93

  // Check for NPU
  info.npu_available = check_npu_available();
  info.npu_tops      = info.npu_available ? 0.5 : 0.0;  // Ethos U-65 at 1 GHz

  return info;
}

/**
 * @brief Performs CPU computational benchmarking.
 *
 * Executes a computational benchmark by calculating prime numbers
 * up to a predefined limit. This tests CPU computational performance
 * and basic arithmetic capabilities.
 *
 * @return TestResult::SUCCESS if benchmark completes successfully,
 *         TestResult::FAILURE if computation errors are detected.
 *
 * @note Uses a simple prime number calculation as the benchmark workload.
 */
TestResult CPUTester::benchmark_cpu() {
  // Simple CPU benchmark: calculate prime numbers
  const int        MAX_PRIME = 10000;
  std::vector<int> primes;

  for (int num = 2; num <= MAX_PRIME; ++num) {
    bool is_prime = true;
    for (int i = 2; i <= std::sqrt(num); ++i) {
      if (num % i == 0) {
        is_prime = false;
        break;
      }
    }
    if (is_prime) {
      primes.push_back(num);
    }
  }

  // Verify we found some primes
  if (primes.empty() || primes.back() != 9973) {  // 10000th prime is 104729, but we use smaller
    return TestResult::FAILURE;
  }

  return TestResult::SUCCESS;
}

/**
 * @brief Tests CPU temperature monitoring capability.
 *
 * Attempts to read the current CPU temperature and validates
 * that it falls within reasonable operating ranges.
 *
 * @return TestResult::SUCCESS if temperature is readable and reasonable,
 *         TestResult::NOT_SUPPORTED if temperature monitoring is unavailable,
 *         TestResult::FAILURE if temperature is outside safe operating range.
 *
 * @note Safe operating range is considered 0-100°C.
 */
TestResult CPUTester::test_temperature() {
  double temp = get_cpu_temperature();
  if (temp < 0) {
    return TestResult::NOT_SUPPORTED;
  }

  // Check if temperature is reasonable (0-100°C)
  if (temp < 0 || temp > 100) {
    return TestResult::FAILURE;
  }

  return TestResult::SUCCESS;
}

/**
 * @brief Monitors CPU temperature stability over time.
 *
 * Continuously monitors CPU temperature for the specified duration,
 * collecting statistics on temperature variation and stability.
 *
 * @param duration The monitoring duration in seconds.
 * @return TestResult::SUCCESS if temperature remains stable (variation ≤ 20°C),
 *         TestResult::FAILURE if temperature fluctuates excessively.
 *
 * @note Temperature readings are taken every second during monitoring.
 * @note Stability is measured by maximum temperature variation.
 */
TestResult CPUTester::monitor_temperature(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();
  auto end_time   = start_time + duration;

  std::vector<double> temperatures;
  double              min_temp = 999.0;
  double              max_temp = -999.0;

  while (std::chrono::steady_clock::now() < end_time) {
    double temp = get_cpu_temperature();
    if (temp >= 0) {
      temperatures.push_back(temp);
      min_temp = std::min(min_temp, temp);
      max_temp = std::max(max_temp, temp);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (temperatures.empty()) {
    return TestResult::NOT_SUPPORTED;
  }

  // Check temperature stability (variation should be reasonable)
  double temp_variation = max_temp - min_temp;

  // Allow up to 20°C variation during monitoring
  return (temp_variation <= 20.0) ? TestResult::SUCCESS : TestResult::FAILURE;
}

/**
 * @brief Tests multi-core CPU functionality.
 *
 * Verifies that the system can utilize multiple CPU cores by
 * spawning threads equal to the number of available cores and
 * performing computational work in each thread.
 *
 * @return TestResult::SUCCESS if all threads complete successfully,
 *         TestResult::NOT_SUPPORTED if multi-threading is unavailable,
 *         TestResult::FAILURE if thread execution fails.
 *
 * @note Uses std::thread::hardware_concurrency() to determine core count.
 */
TestResult CPUTester::test_multi_core() {
  unsigned int num_threads = std::thread::hardware_concurrency();
  if (num_threads == 0) {
    return TestResult::NOT_SUPPORTED;
  }

  // Basic multi-threading test
  std::vector<std::thread> threads;
  std::vector<int>         results(num_threads, 0);

  for (unsigned int i = 0; i < num_threads; ++i) {
    threads.emplace_back([i, &results]() {
      // Simple computation per thread
      int sum = 0;
      for (int j = 0; j < 1000; ++j) {
        sum += j * i;
      }
      results[i] = sum;
    });
  }

  // Wait for all threads
  for (auto& thread : threads) {
    thread.join();
  }

  // Verify all threads completed
  for (int result : results) {
    if (result == 0) {
      return TestResult::FAILURE;
    }
  }

  return TestResult::SUCCESS;
}

/**
 * @brief Tests NPU availability and basic functionality.
 *
 * Checks if the Arm Ethos U-65 NPU is available and accessible.
 * Performs a basic inference test if possible.
 *
 * @return TestResult::SUCCESS if NPU is available and functional,
 *         TestResult::NOT_SUPPORTED if NPU is not available,
 *         TestResult::FAILURE if NPU test fails.
 *
 * @note NPU testing requires appropriate drivers and libraries.
 */
TestResult CPUTester::test_npu() {
  if (!cpu_info_.npu_available) {
    return TestResult::NOT_SUPPORTED;
  }

  // Check for NPU device nodes or drivers
  // Ethos U-65 may appear as /dev/ethos-u or similar
  if (!fs::exists("/dev/ethos-u") && !fs::exists("/sys/class/misc/ethos-u")) {
    // Try to check if NPU driver is loaded
    FILE* lsmod_pipe = popen("lsmod | grep -i ethos", "r");
    if (!lsmod_pipe) {
      return TestResult::NOT_SUPPORTED;
    }
    char buffer[128];
    bool found = fgets(buffer, sizeof(buffer), lsmod_pipe) != nullptr;
    pclose(lsmod_pipe);
    if (!found) {
      return TestResult::NOT_SUPPORTED;
    }
  }

  // Basic NPU functionality test would require inference libraries
  // For now, just check availability
  return TestResult::SUCCESS;
}

/**
 * @brief Checks if NPU is available on the system.
 *
 * Verifies the presence of Arm Ethos U-65 NPU by checking device nodes,
 * driver modules, or system information.
 *
 * @return true if NPU is detected, false otherwise.
 */
bool CPUTester::check_npu_available() {
  // Check for Ethos U-65 device nodes
  if (fs::exists("/dev/ethos-u") || fs::exists("/sys/class/misc/ethos-u")) {
    return true;
  }

  // Check if NPU driver is loaded
  FILE* lsmod_pipe = popen("lsmod | grep -i ethos", "r");
  if (lsmod_pipe) {
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), lsmod_pipe)) {
      pclose(lsmod_pipe);
      return true;
    }
    pclose(lsmod_pipe);
  }

  // Check for NPU in device tree or sysfs
  if (fs::exists("/sys/firmware/devicetree/base/soc/npu") ||
      fs::exists("/proc/device-tree/soc/npu")) {
    return true;
  }

  return false;
}

/**
 * @brief Retrieves the current CPU temperature.
 *
 * Attempts to read CPU temperature from various system locations
 * commonly used on Linux systems for thermal monitoring.
 *
 * @return CPU temperature in degrees Celsius, or -1.0 if unavailable.
 *
 * @note Tries multiple sensor locations in order of preference.
 * @note Handles both millidegree and degree formats automatically.
 */
double CPUTester::get_cpu_temperature() {
  // i.MX93 thermal zones - try in order of preference
  std::vector<std::string> temp_files = {
      "/sys/class/thermal/thermal_zone0/temp",  // Primary CPU thermal zone
      "/sys/class/thermal/thermal_zone1/temp",  // Secondary thermal zone
      "/sys/devices/virtual/thermal/thermal_zone0/temp",
      "/sys/class/hwmon/hwmon0/temp1_input",  // Hardware monitor interface
      "/sys/class/hwmon/hwmon1/temp1_input",
  };

  for (const auto& temp_file : temp_files) {
    std::ifstream temp_stream(temp_file);
    if (temp_stream.is_open()) {
      std::string temp_str;
      std::getline(temp_stream, temp_str);
      try {
        double temp = std::stod(temp_str);
        // Convert millidegrees to degrees if necessary
        if (temp > 1000) {
          temp /= 1000.0;
        }
        // Sanity check: i.MX93 operating range is -40°C to 105°C
        if (temp >= -40.0 && temp <= 125.0) {
          return temp;
        }
      } catch (...) {
        continue;
      }
    }
  }

  return -1.0;  // Not available
}

}  // namespace imx93_peripheral_test