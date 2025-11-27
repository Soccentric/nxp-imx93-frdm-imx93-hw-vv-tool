/**
 * @file main.cpp
 * @brief Main application entry point for FRDM-IMX93 peripheral verification tool.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <CLI/CLI.hpp>

#include "camera_tester.h"
#include "cpu_tester.h"
#include "display_tester.h"
#include "form_factor_tester.h"
#include "gpio_tester.h"
#include "gpu_tester.h"
#include "logger.h"
#include "memory_tester.h"
#include "networking_tester.h"
#include "power_tester.h"
#include "storage_tester.h"
#include "usb_tester.h"

using namespace imx93_peripheral_test;

// Factory for creating testers
using TesterFactory = std::function<std::unique_ptr<PeripheralTester>()>;

std::map<std::string, TesterFactory> tester_registry = {
    {"cpu", []() { return std::make_unique<CPUTester>(); }},
    {"gpio", []() { return std::make_unique<GPIOTester>(); }},
    {"camera", []() { return std::make_unique<CameraTester>(); }},
    {"gpu", []() { return std::make_unique<GPUTester>(); }},
    {"memory", []() { return std::make_unique<MemoryTester>(); }},
    {"storage", []() { return std::make_unique<StorageTester>(); }},
    {"display", []() { return std::make_unique<DisplayTester>(); }},
    {"usb", []() { return std::make_unique<USBTester>(); }},
    {"networking", []() { return std::make_unique<NetworkingTester>(); }},
    {"power", []() { return std::make_unique<PowerTester>(); }},
    {"form_factor", []() { return std::make_unique<FormFactorTester>(); }}};

void list_peripherals() {
  std::cout << "Available Peripherals:\n";
  std::cout << "=====================\n";
  for (const auto& pair : tester_registry) {
    auto tester = pair.second();
    std::cout << pair.first << ": " << (tester->is_available() ? "Available" : "Not Available")
              << "\n";
  }
}

int main(int argc, char* argv[]) {
  CLI::App app{"NXP FRDM-IMX93 Hardware Peripheral Verification Tool"};

  bool json_output = false;
  std::string output_file;
  app.add_flag("--json", json_output, "Output results in JSON format");
  app.add_option("--output", output_file, "Write output to file");

  // List subcommand
  auto list_cmd = app.add_subcommand("list", "List all available peripherals");

  // Test subcommand
  auto test_cmd = app.add_subcommand("test", "Run short tests");
  bool test_all = false;
  std::vector<std::string> test_peripherals;
  test_cmd->add_flag("--all", test_all, "Run short tests for all peripherals");
  test_cmd->add_option("peripherals", test_peripherals, "Specific peripherals to test")
      ->expected(0, -1);

  // Monitor subcommand
  auto monitor_cmd = app.add_subcommand("monitor", "Run monitoring tests");
  bool monitor_all = false;
  int monitor_duration = 10;
  std::vector<std::string> monitor_peripherals;
  monitor_cmd->add_flag("--all", monitor_all, "Run monitoring tests for all peripherals");
  monitor_cmd->add_option("--duration", monitor_duration, "Monitoring duration in seconds")
      ->default_val(10);
  monitor_cmd->add_option("peripherals", monitor_peripherals, "Specific peripherals to monitor")
      ->expected(0, -1);

  CLI11_PARSE(app, argc, argv);

  // Setup logging
  if (!output_file.empty() && !json_output) {
    Logger::instance().set_log_file(output_file);
  }

  if (json_output) {
    Logger::instance().set_console_output(false);
  }

  // Handle list command
  if (*list_cmd) {
    list_peripherals();
    return 0;
  }

  std::vector<TestReport> reports;
  int failed_tests = 0;

  auto run_test = [&](const std::string& name, bool is_monitor = false, int duration = 0) {
    if (tester_registry.find(name) == tester_registry.end()) {
      LOG_ERROR("Unknown peripheral: " + name);
      return;
    }

    auto tester = tester_registry[name]();
    if (!tester->is_available()) {
      LOG_WARN(name + ": Not available, skipping...");
      return;
    }

    TestReport report;
    if (is_monitor) {
      LOG_INFO("Running monitoring test for " + name + " (" + std::to_string(duration) + "s)...");
      report = tester->monitor_test(std::chrono::seconds(duration));
    } else {
      LOG_INFO("Running short test for " + name + "...");
      report = tester->short_test();
    }

    reports.push_back(report);

    if (!json_output) {
      LOG_INFO("Result: " + test_result_to_string(report.result));
      LOG_INFO("Details: " + report.details);
    }

    if (report.result != TestResult::SUCCESS) {
      failed_tests++;
    }
  };

  // Handle test command
  if (*test_cmd) {
    if (test_all) {
      for (const auto& pair : tester_registry) {
        run_test(pair.first, false);
      }
    } else if (!test_peripherals.empty()) {
      for (const auto& peripheral : test_peripherals) {
        run_test(peripheral, false);
      }
    } else {
      std::cout << "Error: Specify --all or provide peripheral names for test command\n";
      return 1;
    }
  }

  // Handle monitor command
  if (*monitor_cmd) {
    if (monitor_all) {
      for (const auto& pair : tester_registry) {
        run_test(pair.first, true, monitor_duration);
      }
    } else if (!monitor_peripherals.empty()) {
      for (const auto& peripheral : monitor_peripherals) {
        run_test(peripheral, true, monitor_duration);
      }
    } else {
      std::cout << "Error: Specify --all or provide peripheral names for monitor command\n";
      return 1;
    }
  }

  // If no subcommand was used, show help
  if (!*list_cmd && !*test_cmd && !*monitor_cmd) {
    std::cout << app.help() << std::endl;
    return 1;
  }

  if (json_output) {
    std::stringstream json_ss;
    json_ss << "{\"tests\": [";
    for (size_t i = 0; i < reports.size(); ++i) {
      json_ss << reports[i].to_json();
      if (i < reports.size() - 1)
        json_ss << ",";
    }
    json_ss << "], \"summary\": {";
    json_ss << "\"total\": " << reports.size() << ",";
    json_ss << "\"failed\": " << failed_tests << ",";
    json_ss << "\"passed\": " << (reports.size() - failed_tests);
    json_ss << "}}";

    if (!output_file.empty()) {
      std::ofstream out(output_file);
      out << json_ss.str();
    } else {
      std::cout << json_ss.str() << std::endl;
    }
  }

  return failed_tests == 0 ? 0 : 1;
}
