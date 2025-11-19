/**
 * @file main.cpp
 * @brief Main application entry point for FRDM-IMX93 peripheral verification tool.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include "gpio_tester.h"
#include "cpu_tester.h"
#include "camera_tester.h"
#include "gpu_tester.h"
#include "memory_tester.h"
#include "storage_tester.h"
#include "display_tester.h"
#include "usb_tester.h"
#include "networking_tester.h"
#include "power_tester.h"
#include "form_factor_tester.h"
#include "logger.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <map>
#include <functional>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace imx93_peripheral_test;

// Configuration structure
struct Config {
    bool json_output = false;
    std::string output_file;
    bool list_peripherals = false;
    bool run_all_short = false;
    int monitor_duration = 0;
    std::vector<std::string> specific_tests;
    bool short_test = false;
};

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
    {"form_factor", []() { return std::make_unique<FormFactorTester>(); }}
};

void print_usage(const char* program_name) {
    std::cout << "NXP FRDM-IMX93 Hardware Peripheral Verification Tool\n"
              << "Usage: " << program_name << " [options]\n\n"
              << "Options:\n"
              << "  --all-short          Run short tests for all peripherals\n"
              << "  --all-monitor <sec>  Run monitoring tests for all peripherals\n"
              << "  --test <peripheral>  Run short test for specific peripheral\n"
              << "  --monitor <peripheral> <sec> Run monitoring test for specific peripheral\n"
              << "  --list               List all available peripherals\n"
              << "  --json               Output results in JSON format\n"
              << "  --output <file>      Write output to file\n"
              << "  --help               Show this help message\n\n"
              << "Available peripherals: cpu, gpio, camera, gpu, memory, storage, display, usb, networking, power, form_factor\n";
}

void list_peripherals() {
    std::cout << "Available Peripherals:\n";
    std::cout << "=====================\n";
    for (const auto& pair : tester_registry) {
        auto tester = pair.second();
        std::cout << pair.first << ": " << (tester->is_available() ? "Available" : "Not Available") << "\n";
    }
}

int main(int argc, char* argv[]) {
    Config config;
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse arguments
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (args[i] == "--list") {
            config.list_peripherals = true;
        } else if (args[i] == "--json") {
            config.json_output = true;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            config.output_file = args[++i];
        } else if (args[i] == "--all-short") {
            config.run_all_short = true;
        } else if (args[i] == "--all-monitor" && i + 1 < args.size()) {
            config.monitor_duration = std::stoi(args[++i]);
            for (const auto& pair : tester_registry) {
                config.specific_tests.push_back(pair.first);
            }
        } else if (args[i] == "--test" && i + 1 < args.size()) {
            config.specific_tests.push_back(args[++i]);
            config.short_test = true;
        } else if (args[i] == "--monitor" && i + 2 < args.size()) {
            config.specific_tests.push_back(args[++i]);
            config.monitor_duration = std::stoi(args[++i]);
        }
    }

    // Setup logging
    if (!config.output_file.empty() && !config.json_output) {
        Logger::instance().set_log_file(config.output_file);
    }

    if (config.json_output) {
        Logger::instance().set_console_output(false);
    }

    if (config.list_peripherals) {
        list_peripherals();
        return 0;
    }

    std::vector<TestReport> reports;
    int failed_tests = 0;

    auto run_test = [&](const std::string& name) {
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
        if (config.monitor_duration > 0) {
            LOG_INFO("Running monitoring test for " + name + " (" + std::to_string(config.monitor_duration) + "s)...");
            report = tester->monitor_test(std::chrono::seconds(config.monitor_duration));
        } else {
            LOG_INFO("Running short test for " + name + "...");
            report = tester->short_test();
        }

        reports.push_back(report);
        
        if (!config.json_output) {
            LOG_INFO("Result: " + test_result_to_string(report.result));
            LOG_INFO("Details: " + report.details);
        }

        if (report.result != TestResult::SUCCESS) {
            failed_tests++;
        }
    };

    if (config.run_all_short) {
        for (const auto& pair : tester_registry) {
            run_test(pair.first);
        }
    } else {
        for (const auto& test : config.specific_tests) {
            run_test(test);
        }
    }

    if (config.json_output) {
        std::stringstream json_ss;
        json_ss << "{\"tests\": [";
        for (size_t i = 0; i < reports.size(); ++i) {
            json_ss << reports[i].to_json();
            if (i < reports.size() - 1) json_ss << ",";
        }
        json_ss << "], \"summary\": {";
        json_ss << "\"total\": " << reports.size() << ",";
        json_ss << "\"failed\": " << failed_tests << ",";
        json_ss << "\"passed\": " << (reports.size() - failed_tests);
        json_ss << "}}";

        if (!config.output_file.empty()) {
            std::ofstream out(config.output_file);
            out << json_ss.str();
        } else {
            std::cout << json_ss.str() << std::endl;
        }
    }

    return failed_tests == 0 ? 0 : 1;
}
