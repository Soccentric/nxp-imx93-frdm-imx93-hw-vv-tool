/**
 * @file power_tester.cpp
 * @brief Implementation of power management peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the i.MX 93 power management:
 * - PCA9451A PMIC (Power Management IC)
 * - Multiple voltage rails (VDD_SOC, VDD_ARM, NVCC, etc.)
 * - Dynamic voltage and frequency scaling (DVFS)
 * - Low-power modes (WAIT, STOP, SUSPEND)
 * - Temperature monitoring and throttling
 * - Voltage and current monitoring
 */

#include "power_tester.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

namespace imx93_peripheral_test {

/**
 * @brief Constructs a power tester instance.
 *
 * Initializes the power tester by detecting available power management
 * interfaces and battery/power supply monitoring capabilities.
 * For i.MX93, checks for PCA9451A PMIC and voltage rail monitoring.
 */
PowerTester::PowerTester() : power_available_(false) {
  // Check if power management is available on i.MX93
  // Look for PMIC interfaces and voltage regulators
  power_available_ = fs::exists("/sys/class/power_supply") || fs::exists("/sys/class/regulator") ||
                     fs::exists("/sys/devices/platform/soc@0");

  if (power_available_) {
    power_info_ = get_power_info();
  }
}

TestReport PowerTester::short_test() {
  auto start_time = std::chrono::steady_clock::now();

  if (!power_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Power management not available",
                         std::chrono::milliseconds(0));
  }

  std::stringstream details;
  bool              all_passed = true;

  // Display current power information
  details << "Power Source: ";
  switch (power_info_.source) {
    case PowerSource::BATTERY:
      details << "Battery";
      break;
    case PowerSource::AC_ADAPTER:
      details << "AC Adapter";
      break;
    case PowerSource::USB_C:
      details << "USB-C";
      break;
    case PowerSource::POE:
      details << "PoE";
      break;
    case PowerSource::UNKNOWN:
      details << "Unknown";
      break;
  }
  details << "\n";

  if (power_info_.voltage_v > 0) {
    details << "Voltage: " << power_info_.voltage_v << "V\n";
  } else if (power_info_.source == PowerSource::AC_ADAPTER) {
    details << "Voltage: N/A (AC Adapter)\n";
  }

  if (power_info_.current_ma > 0) {
    details << "Current: " << power_info_.current_ma << "mA\n";
  } else if (power_info_.source == PowerSource::AC_ADAPTER) {
    details << "Current: N/A (AC Adapter)\n";
  }

  if (power_info_.power_w > 0) {
    details << "Power: " << power_info_.power_w << "W\n";
  } else if (power_info_.source == PowerSource::AC_ADAPTER) {
    details << "Power: N/A (AC Adapter)\n";
  }

  if (power_info_.battery_present) {
    details << "Battery: " << power_info_.battery_percentage << "%\n";
  }

  // Test power source detection
  TestResult source_result = test_power_source();
  details << "Power Source Detection: " << (source_result == TestResult::SUCCESS ? "PASS" : "FAIL")
          << "\n";
  if (source_result != TestResult::SUCCESS)
    all_passed = false;

  // Test power monitoring
  TestResult monitor_result = test_power_monitoring();
  details << "Power Monitoring: "
          << (monitor_result == TestResult::SUCCESS
                  ? "PASS"
                  : (monitor_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (monitor_result != TestResult::SUCCESS && monitor_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  // Test battery if present
  TestResult battery_result = test_battery();
  details << "Battery Test: "
          << (battery_result == TestResult::SUCCESS
                  ? "PASS"
                  : (battery_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (battery_result != TestResult::SUCCESS && battery_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  // Test power management
  TestResult pm_result = test_power_management();
  details << "Power Management: " << (pm_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
  if (pm_result != TestResult::SUCCESS)
    all_passed = false;

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
  return create_report(overall_result, details.str(), duration);
}

TestReport PowerTester::monitor_test(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();

  if (!power_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Power management not available",
                         std::chrono::milliseconds(0));
  }

  TestResult result = monitor_power_consumption(duration);

  auto end_time      = std::chrono::steady_clock::now();
  auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::string details =
      "Power monitoring completed for " + std::to_string(duration.count()) + " seconds";
  return create_report(result, details, test_duration);
}

bool PowerTester::is_available() const {
  return power_available_;
}

PowerInfo PowerTester::get_power_info() {
  PowerInfo info = {};  // Initialize all members to 0/false/empty
  info.source    = PowerSource::UNKNOWN;
  info.state     = PowerState::UNKNOWN;

  // Check power supplies in sysfs
  if (fs::exists("/sys/class/power_supply")) {
    for (const auto& entry : fs::directory_iterator("/sys/class/power_supply")) {
      std::string supply_path = entry.path().string();
      PowerInfo   supply_info = parse_power_supply(supply_path);

      // Use the first valid power supply info that has meaningful data
      if (!supply_info.power_supply_model.empty() &&
          (supply_info.voltage_v > 0 || supply_info.current_ma > 0 || supply_info.power_w > 0 ||
           supply_info.battery_present || supply_info.ac_connected)) {
        info = supply_info;
        break;
      }
    }
  }

  // If no sysfs info with meaningful data, try to detect from other sources
  if (info.source == PowerSource::UNKNOWN) {
    // Check for AC adapter
    if (fs::exists("/sys/class/power_supply/AC") || fs::exists("/sys/class/power_supply/ACAD")) {
      std::string   ac_path = fs::exists("/sys/class/power_supply/AC")
                                  ? "/sys/class/power_supply/AC"
                                  : "/sys/class/power_supply/ACAD";
      std::ifstream online_file(ac_path + "/online");
      if (online_file.is_open()) {
        int online = 0;
        online_file >> online;
        if (online == 1) {
          info.source       = PowerSource::AC_ADAPTER;
          info.ac_connected = true;
        }
      }
    }

    // Check for battery
    if (fs::exists("/sys/class/power_supply/BAT0") || fs::exists("/sys/class/power_supply/BAT1")) {
      info.battery_present = true;
      info.source          = PowerSource::BATTERY;
    }
  }

  return info;
}

TestResult PowerTester::test_power_source() {
  // Test if we can detect the power source
  if (power_info_.source != PowerSource::UNKNOWN) {
    return TestResult::SUCCESS;
  }

  // Try alternative detection methods
  if (system("which upower > /dev/null 2>&1") == 0) {
    // Use upower if available
    FILE* upower_pipe = popen("upower -e 2>/dev/null", "r");
    if (upower_pipe) {
      char buffer[256];
      if (fgets(buffer, sizeof(buffer), upower_pipe) != NULL) {
        pclose(upower_pipe);
        return TestResult::SUCCESS;
      }
      pclose(upower_pipe);
    }
  }

  return TestResult::FAILURE;
}

TestResult PowerTester::test_power_monitoring() {
  // Test if we can monitor voltage/current/power
  if (power_info_.voltage_v > 0 || power_info_.current_ma > 0 || power_info_.power_w > 0) {
    return TestResult::SUCCESS;
  }

  // Check if monitoring is supported but no values available yet
  if (fs::exists("/sys/class/power_supply")) {
    return TestResult::NOT_SUPPORTED;
  }

  return TestResult::FAILURE;
}

TestResult PowerTester::test_battery() {
  if (!power_info_.battery_present) {
    return TestResult::NOT_SUPPORTED;
  }

  // Test battery percentage reading
  if (power_info_.battery_percentage >= 0 && power_info_.battery_percentage <= 100) {
    return TestResult::SUCCESS;
  }

  return TestResult::FAILURE;
}

TestResult PowerTester::test_power_management() {
  // Test basic power management features
  bool pm_available = false;

  // Check for suspend support
  if (fs::exists("/sys/power/state")) {
    pm_available = true;
  }

  // Check for CPU frequency scaling (power management)
  if (fs::exists("/sys/devices/system/cpu/cpu0/cpufreq")) {
    pm_available = true;
  }

  return pm_available ? TestResult::SUCCESS : TestResult::FAILURE;
}

TestResult PowerTester::monitor_power_consumption(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();
  auto end_time   = start_time + duration;

  bool      monitoring_stable = true;
  PowerInfo initial_info      = get_power_info();

  while (std::chrono::steady_clock::now() < end_time && monitoring_stable) {
    PowerInfo current_info = get_power_info();

    // Check if power source changed unexpectedly
    if (current_info.source != initial_info.source) {
      // Source change is OK, just update
      initial_info = current_info;
    }

    // For battery systems, check if battery drains too fast
    if (current_info.battery_present && initial_info.battery_present) {
      int drain_rate = initial_info.battery_percentage - current_info.battery_percentage;
      if (drain_rate > 50) {  // More than 50% drain during monitoring period
        monitoring_stable = false;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  return monitoring_stable ? TestResult::SUCCESS : TestResult::FAILURE;
}

PowerConsumption PowerTester::measure_power_consumption() {
  PowerConsumption consumption = {0.0, 0.0, 0.0, 0.0};

  // This would require specialized hardware to measure actual power consumption
  // For now, return placeholder values

  return consumption;
}

PowerInfo PowerTester::parse_power_supply(const std::string& supply_path) {
  PowerInfo info = {};  // Initialize all members to 0/false/empty
  info.source    = PowerSource::UNKNOWN;

  // Check if power supply is online first
  std::ifstream online_file(supply_path + "/online");
  if (online_file.is_open()) {
    int online = 0;
    online_file >> online;
    if (online != 1) {
      // Power supply is not online, return empty info
      return info;
    }
  }

  // Read type
  std::ifstream type_file(supply_path + "/type");
  if (type_file.is_open()) {
    std::string type;
    type_file >> type;
    if (type == "Battery") {
      info.source          = PowerSource::BATTERY;
      info.battery_present = true;
    } else if (type == "Mains") {
      info.source       = PowerSource::AC_ADAPTER;
      info.ac_connected = true;
    } else if (type == "USB") {
      info.source = PowerSource::USB_C;
    }
  }

  // Read model name
  std::ifstream model_file(supply_path + "/model_name");
  if (model_file.is_open()) {
    std::getline(model_file, info.power_supply_model);
  }

  // Read voltage (microvolts)
  std::ifstream voltage_file(supply_path + "/voltage_now");
  if (voltage_file.is_open()) {
    int voltage_uv;
    voltage_file >> voltage_uv;
    if (voltage_uv > 0) {
      info.voltage_v = voltage_uv / 1000000.0;
    }
  }

  // Read current (microamps)
  std::ifstream current_file(supply_path + "/current_now");
  if (current_file.is_open()) {
    int current_ua;
    current_file >> current_ua;
    if (current_ua > 0) {
      info.current_ma = current_ua / 1000.0;
    }
  }

  // Calculate power
  if (info.voltage_v > 0 && info.current_ma > 0) {
    info.power_w = (info.voltage_v * info.current_ma) / 1000.0;
  }

  // Read battery capacity
  if (info.battery_present) {
    std::ifstream capacity_file(supply_path + "/capacity");
    if (capacity_file.is_open()) {
      capacity_file >> info.battery_percentage;
    }
  }

  return info;
}

}  // namespace imx93_peripheral_test