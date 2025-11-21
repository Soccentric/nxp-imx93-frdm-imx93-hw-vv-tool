/**
 * @file memory_tester.cpp
 * @brief Implementation of Memory peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the i.MX 93 memory subsystem:
 * - DDR4 or LPDDR4/LPDDR4X support (up to 2GB on FRDM board)
 * - Advanced memory controller with ECC support
 * - Memory bandwidth testing
 * - Integrity verification with various patterns
 */

#include "memory_tester.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <thread>

namespace fs = std::filesystem;

namespace imx93_peripheral_test {

/**
 * @brief Constructs a Memory tester instance.
 *
 * Initializes the Memory tester by checking for memory information availability
 * and retrieving initial memory information from /proc/meminfo.
 * For i.MX93, the FRDM board typically has 2GB DDR4 or LPDDR4.
 */
MemoryTester::MemoryTester() : memory_available_(false) {
  // Check if memory information is available
  memory_available_ = fs::exists("/proc/meminfo");
  if (memory_available_) {
    memory_info_ = get_memory_info();
  }
}

/**
 * @brief Performs short verification test of memory functionality.
 *
 * Executes a comprehensive short test that covers:
 * - Memory information retrieval and validation
 * - RAM integrity testing with various data patterns
 * - Memory bandwidth performance testing
 * - ECC functionality verification (if supported)
 *
 * @return TestReport containing detailed test results and timing information.
 *
 * @note This test provides a quick assessment of memory subsystem health.
 */
TestReport MemoryTester::short_test() {
  auto start_time = std::chrono::steady_clock::now();

  if (!memory_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Memory information not available",
                         std::chrono::milliseconds(0));
  }

  std::stringstream details;
  bool              all_passed = true;

  details << "Total RAM: " << memory_info_.total_ram_mb << " MB\n";
  details << "Available RAM: " << memory_info_.available_ram_mb << " MB\n";
  details << "Memory Type: " << memory_info_.memory_type << "\n";
  details << "ECC Supported: " << (memory_info_.ecc_supported ? "Yes" : "No") << "\n";
  details << "ECC Enabled: " << (memory_info_.ecc_enabled ? "Yes" : "No") << "\n";

  // Test RAM integrity
  TestResult integrity_result = test_ram_integrity();
  details << "RAM Integrity: " << (integrity_result == TestResult::SUCCESS ? "PASS" : "FAIL")
          << "\n";
  if (integrity_result != TestResult::SUCCESS)
    all_passed = false;

  // Test memory bandwidth
  TestResult bandwidth_result = test_memory_bandwidth();
  details << "Memory Bandwidth: " << (bandwidth_result == TestResult::SUCCESS ? "PASS" : "FAIL")
          << "\n";
  if (bandwidth_result != TestResult::SUCCESS)
    all_passed = false;

  // Test ECC if supported
  TestResult ecc_result = test_ecc();
  details << "ECC Test: "
          << (ecc_result == TestResult::SUCCESS
                  ? "PASS"
                  : (ecc_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (ecc_result != TestResult::SUCCESS && ecc_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
  return create_report(overall_result, details.str(), duration);
}

/**
 * @brief Performs extended monitoring of memory peripherals.
 *
 * Monitors memory usage over a specified duration to detect:
 * - Memory leaks
 * - Excessive memory consumption
 * - Memory usage stability
 *
 * @param duration The time period over which to monitor memory functionality.
 * @return TestReport containing monitoring results and memory usage statistics.
 *
 * @note This is a long-running test that monitors memory usage over time.
 */
TestReport MemoryTester::monitor_test(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();

  if (!memory_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Memory information not available",
                         std::chrono::milliseconds(0));
  }

  TestResult result = monitor_memory_usage(duration);

  auto end_time      = std::chrono::steady_clock::now();
  auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::string details =
      "Memory monitoring completed for " + std::to_string(duration.count()) + " seconds";
  return create_report(result, details, test_duration);
}

/**
 * @brief Checks if memory peripheral is available on the current system.
 *
 * Verifies the presence of /proc/meminfo which is required for
 * memory information retrieval on Linux systems.
 *
 * @return true if memory information is accessible, false otherwise.
 */
bool MemoryTester::is_available() const {
  return memory_available_;
}

/**
 * @brief Retrieves comprehensive memory information from system files.
 *
 * Parses /proc/meminfo and uses dmidecode to gather detailed
 * information about the system's memory including total RAM,
 * available RAM, memory type, and ECC capabilities.
 *
 * @return MemoryInfo structure containing all retrieved memory information.
 *
 * @note This function caches the memory information for performance.
 * @note dmidecode requires root privileges for detailed memory information.
 */
MemoryInfo MemoryTester::get_memory_info() {
  MemoryInfo    info;
  std::ifstream meminfo("/proc/meminfo");

  if (!meminfo.is_open()) {
    return info;
  }

  std::string line;
  while (std::getline(meminfo, line)) {
    if (line.find("MemTotal:") != std::string::npos) {
      std::stringstream ss(line);
      std::string       label, value, unit;
      ss >> label >> value >> unit;
      info.total_ram_mb = std::stoull(value) / 1024;
    } else if (line.find("MemAvailable:") != std::string::npos) {
      std::stringstream ss(line);
      std::string       label, value, unit;
      ss >> label >> value >> unit;
      info.available_ram_mb = std::stoull(value) / 1024;
    }
  }

  // Try to get memory type from dmidecode (requires root)
  FILE* dmidecode_pipe = popen(
      "dmidecode -t memory 2>/dev/null | grep -A 10 'Memory Device' | grep 'Type:' | head -1", "r");
  if (dmidecode_pipe) {
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), dmidecode_pipe)) {
      std::string line(buffer);
      size_t      colon_pos = line.find(':');
      if (colon_pos != std::string::npos) {
        info.memory_type = line.substr(colon_pos + 2);
        info.memory_type.erase(info.memory_type.find_last_not_of("\n\r\t") + 1);
      }
    }
    pclose(dmidecode_pipe);
  }

  // Check for ECC support
  std::ifstream ecc_file("/sys/devices/system/edac/mc/mc0/ecc_capability");
  info.ecc_supported = ecc_file.is_open();
  if (info.ecc_supported) {
    std::string ecc_status;
    std::getline(ecc_file, ecc_status);
    info.ecc_enabled = (ecc_status.find("enabled") != std::string::npos);
  }

  return info;
}

/**
 * @brief Tests RAM integrity with various data patterns.
 *
 * Performs comprehensive RAM integrity testing using multiple test patterns:
 * - All zeros pattern
 * - All ones pattern
 * - Alternating bits pattern
 * - Random data pattern
 *
 * This ensures that memory cells can reliably store and retrieve
 * different types of data without corruption.
 *
 * @return TestResult::SUCCESS if all integrity tests pass,
 *         TestResult::FAILURE if any data corruption is detected.
 *
 * @note Uses a 1MB test buffer to balance thoroughness with performance.
 */
TestResult MemoryTester::test_ram_integrity() {
  // Test memory integrity with different patterns
  const size_t         test_size = 1024 * 1024;  // 1MB test
  std::vector<uint8_t> test_buffer(test_size);

  // Test pattern 1: All zeros
  std::fill(test_buffer.begin(), test_buffer.end(), 0);
  if (!std::all_of(test_buffer.begin(), test_buffer.end(), [](uint8_t val) { return val == 0; })) {
    return TestResult::FAILURE;
  }

  // Test pattern 2: All ones
  std::fill(test_buffer.begin(), test_buffer.end(), 255);
  if (!std::all_of(test_buffer.begin(), test_buffer.end(),
                   [](uint8_t val) { return val == 255; })) {
    return TestResult::FAILURE;
  }

  // Test pattern 3: Alternating bits
  for (size_t i = 0; i < test_size; ++i) {
    test_buffer[i] = (i % 2 == 0) ? 0xAA : 0x55;
  }
  for (size_t i = 0; i < test_size; ++i) {
    uint8_t expected = (i % 2 == 0) ? 0xAA : 0x55;
    if (test_buffer[i] != expected) {
      return TestResult::FAILURE;
    }
  }

  // Test pattern 4: Random data
  std::random_device                     rd;
  std::mt19937                           gen(rd());
  std::uniform_int_distribution<uint8_t> dist(0, 255);

  std::vector<uint8_t> original_data(test_size);
  for (size_t i = 0; i < test_size; ++i) {
    original_data[i] = dist(gen);
    test_buffer[i]   = original_data[i];
  }

  if (!std::equal(test_buffer.begin(), test_buffer.end(), original_data.begin())) {
    return TestResult::FAILURE;
  }

  return TestResult::SUCCESS;
}

/**
 * @brief Tests memory bandwidth performance.
 *
 * Performs a simple memory bandwidth test by timing sequential
 * read and write operations on a large memory buffer.
 *
 * @return TestResult::SUCCESS if bandwidth test completes within time limits,
 *         TestResult::FAILURE if memory operations are too slow.
 *
 * @note Uses a 100MB buffer for the bandwidth test.
 * @note Performance threshold is set at 5 seconds for 100MB operations.
 */
TestResult MemoryTester::test_memory_bandwidth() {
  // Simple memory bandwidth test
  const size_t         test_size = 100 * 1024 * 1024;  // 100MB
  std::vector<uint8_t> buffer(test_size);

  auto start_time = std::chrono::high_resolution_clock::now();

  // Sequential write
  for (size_t i = 0; i < test_size; ++i) {
    buffer[i] = static_cast<uint8_t>(i % 256);
  }

  // Sequential read
  volatile uint8_t sum = 0;
  for (size_t i = 0; i < test_size; ++i) {
    sum += buffer[i];
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  // Basic check - if it took too long, consider it failed
  if (duration.count() > 5000) {  // More than 5 seconds for 100MB
    return TestResult::FAILURE;
  }

  return TestResult::SUCCESS;
}

/**
 * @brief Tests ECC (Error-Correcting Code) functionality.
 *
 * Verifies ECC support and functionality if available on the system.
 * Checks ECC status and looks for any existing ECC errors.
 *
 * @return TestResult::SUCCESS if ECC is enabled and no errors detected,
 *         TestResult::NOT_SUPPORTED if ECC is not supported,
 *         TestResult::FAILURE if ECC is supported but disabled or has errors.
 *
 * @note ECC testing requires ECC-capable memory and motherboard support.
 */
TestResult MemoryTester::test_ecc() {
  if (!memory_info_.ecc_supported) {
    return TestResult::NOT_SUPPORTED;
  }

  // Check ECC status
  if (!memory_info_.ecc_enabled) {
    return TestResult::FAILURE;
  }

  // Check for ECC errors
  std::ifstream ecc_errors("/sys/devices/system/edac/mc/mc0/ce_count");
  if (ecc_errors.is_open()) {
    std::string error_count_str;
    std::getline(ecc_errors, error_count_str);
    try {
      int error_count = std::stoi(error_count_str);
      if (error_count > 0) {
        return TestResult::FAILURE;
      }
    } catch (...) {
      return TestResult::FAILURE;
    }
  }

  return TestResult::SUCCESS;
}

/**
 * @brief Monitors memory usage over time.
 *
 * Continuously monitors system memory usage for the specified duration,
 * collecting statistics on memory consumption patterns.
 *
 * @param duration The monitoring duration in seconds.
 * @return TestResult::SUCCESS if memory usage remains stable (≤10% variation),
 *         TestResult::FAILURE if memory usage fluctuates excessively.
 *
 * @note Memory usage readings are taken every second during monitoring.
 * @note Stability is measured by maximum usage variation as percentage of total RAM.
 */
TestResult MemoryTester::monitor_memory_usage(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();
  auto end_time   = start_time + duration;

  std::vector<uint64_t> memory_usage;
  uint64_t              min_usage = UINT64_MAX;
  uint64_t              max_usage = 0;

  while (std::chrono::steady_clock::now() < end_time) {
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
      std::string line;
      while (std::getline(meminfo, line)) {
        if (line.find("MemAvailable:") != std::string::npos) {
          std::stringstream ss(line);
          std::string       label, value, unit;
          ss >> label >> value >> unit;
          uint64_t available_mb = std::stoull(value) / 1024;
          uint64_t used_mb      = memory_info_.total_ram_mb - available_mb;

          memory_usage.push_back(used_mb);
          min_usage = std::min(min_usage, used_mb);
          max_usage = std::max(max_usage, used_mb);
          break;
        }
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  if (memory_usage.empty()) {
    return TestResult::FAILURE;
  }

  // Check for memory leaks (usage increase over time)
  double usage_variation = static_cast<double>(max_usage - min_usage) / memory_info_.total_ram_mb;
  return (usage_variation <= 0.1) ? TestResult::SUCCESS
                                  : TestResult::FAILURE;  // Allow 10% variation
}

/**
 * @brief Performs stress testing of memory with large allocations.
 *
 * Allocates and tests a large block of memory to stress-test the
 * memory subsystem and verify stability under high memory usage.
 *
 * @param test_size_mb The size of the memory block to allocate and test (in MB).
 * @return TestResult::SUCCESS if stress test passes,
 *         TestResult::NOT_SUPPORTED if insufficient memory available,
 *         TestResult::FAILURE if memory allocation or testing fails.
 *
 * @note Uses up to 80% of available RAM to avoid system instability.
 */
TestResult MemoryTester::stress_test_memory(size_t test_size_mb) {
  // Allocate and test large memory blocks
  size_t test_size_bytes = test_size_mb * 1024 * 1024;

  // Check if we have enough available memory
  if (test_size_bytes >
      memory_info_.available_ram_mb * 1024 * 1024 * 0.8) {  // Use 80% of available
    return TestResult::NOT_SUPPORTED;
  }

  try {
    std::vector<uint8_t> stress_buffer(test_size_bytes);

    // Fill with pattern
    for (size_t i = 0; i < test_size_bytes; ++i) {
      stress_buffer[i] = static_cast<uint8_t>(i % 256);
    }

    // Verify pattern
    for (size_t i = 0; i < test_size_bytes; ++i) {
      if (stress_buffer[i] != static_cast<uint8_t>(i % 256)) {
        return TestResult::FAILURE;
      }
    }

  } catch (const std::bad_alloc&) {
    return TestResult::FAILURE;
  }

  return TestResult::SUCCESS;
}

}  // namespace imx93_peripheral_test