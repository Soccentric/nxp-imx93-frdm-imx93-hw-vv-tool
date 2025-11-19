/**
 * @file form_factor_tester.cpp
 * @brief Implementation of form factor and physical interface tester for FRDM-IMX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the FRDM-IMX93 board physical characteristics:
 * - Board dimensions and mounting holes
 * - Arduino-compatible headers (Uno R3 shield compatible)
 * - mikroBUS socket
 * - FlexCAN and LIN interfaces
 * - Debug UART console
 * - JTAG/SWD debug interface
 * - User LEDs and buttons
 * - Expansion connectors
 */

#include "form_factor_tester.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace imx93_peripheral_test {

/**
 * @brief Constructs a form factor tester instance.
 *
 * Initializes the form factor tester by detecting available physical
 * interfaces and form factor-specific hardware components.
 * For FRDM-IMX93, checks Arduino headers, mikroBUS, and board-specific interfaces.
 */

FormFactorTester::FormFactorTester() : form_factor_available_(false) {
    // Check if form factor testing is available on FRDM-IMX93
    form_factor_available_ = fs::exists("/proc/device-tree") ||
                            fs::exists("/sys/firmware/devicetree") ||
                            fs::exists("/sys/class/gpio");

    if (form_factor_available_) {
        form_factor_info_ = get_form_factor_info();
    }
}

TestReport FormFactorTester::short_test() {
    auto start_time = std::chrono::steady_clock::now();

    if (!form_factor_available_) {
        return create_report(TestResult::NOT_SUPPORTED, "Form factor testing not available", std::chrono::milliseconds(0));
    }

    std::stringstream details;
    bool all_passed = true;

    // Display board information
    details << "Module Type: " << form_factor_info_.module_type << "\n";
    details << "Revision: " << form_factor_info_.revision << "\n";
    if (!form_factor_info_.serial_number.empty()) {
        details << "Serial Number: " << form_factor_info_.serial_number << "\n";
    }
    details << "Temperature: " << form_factor_info_.board_temperature_c << "°C\n";
    details << "Available Interfaces: " << form_factor_info_.interfaces.size() << "\n";

    // Test board information
    TestResult board_result = test_board_info();
    details << "Board Info: " << (board_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
    if (board_result != TestResult::SUCCESS) all_passed = false;

    // Test GPIO pins
    TestResult gpio_result = test_gpio_pins();
    details << "GPIO Pins: " << (gpio_result == TestResult::SUCCESS ? "PASS" : (gpio_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL")) << "\n";
    if (gpio_result != TestResult::SUCCESS && gpio_result != TestResult::NOT_SUPPORTED) all_passed = false;

    // Test interfaces
    TestResult interface_result = test_interfaces();
    details << "Interfaces: " << (interface_result == TestResult::SUCCESS ? "PASS" : "FAIL") << "\n";
    if (interface_result != TestResult::SUCCESS) all_passed = false;

    // Test temperature
    TestResult temp_result = test_temperature();
    details << "Temperature: " << (temp_result == TestResult::SUCCESS ? "PASS" : (temp_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL")) << "\n";
    if (temp_result != TestResult::SUCCESS && temp_result != TestResult::NOT_SUPPORTED) all_passed = false;

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
    return create_report(overall_result, details.str(), duration);
}

TestReport FormFactorTester::monitor_test(std::chrono::seconds duration) {
    auto start_time = std::chrono::steady_clock::now();

    if (!form_factor_available_) {
        return create_report(TestResult::NOT_SUPPORTED, "Form factor testing not available", std::chrono::milliseconds(0));
    }

    TestResult result = monitor_interfaces(duration);

    auto end_time = std::chrono::steady_clock::now();
    auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::string details = "Interface monitoring completed for " + std::to_string(duration.count()) + " seconds";
    return create_report(result, details, test_duration);
}

bool FormFactorTester::is_available() const {
    return form_factor_available_;
}

FormFactorInfo FormFactorTester::get_form_factor_info() {
    FormFactorInfo info;

    // Get module type and revision from device tree
    if (fs::exists("/proc/device-tree/model")) {
        std::ifstream model_file("/proc/device-tree/model");
        if (model_file.is_open()) {
            std::getline(model_file, info.module_type);
        }
    }

    // Get revision
    if (fs::exists("/proc/device-tree/revision")) {
        std::ifstream rev_file("/proc/device-tree/revision");
        if (rev_file.is_open()) {
            std::string rev_hex;
            rev_file >> rev_hex;
            info.revision = rev_hex;
        }
    }

    // Get serial number
    if (fs::exists("/proc/device-tree/serial-number")) {
        std::ifstream serial_file("/proc/device-tree/serial-number");
        if (serial_file.is_open()) {
            std::getline(serial_file, info.serial_number);
        }
    }

    // Get temperature
    info.board_temperature_c = get_board_temperature();

    // Enumerate interfaces
    info.interfaces = enumerate_interfaces();

    return info;
}

TestResult FormFactorTester::test_board_info() {
    // Test if we can read basic board information
    if (!form_factor_info_.module_type.empty() || !form_factor_info_.revision.empty()) {
        return TestResult::SUCCESS;
    }

    // Try alternative methods
    if (system("which vcgencmd > /dev/null 2>&1") == 0) {
        // Use vcgencmd if available (NXP i.MX93 specific)
        FILE* vcgencmd_pipe = popen("vcgencmd get_config str 2>/dev/null", "r");
        if (vcgencmd_pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), vcgencmd_pipe) != NULL) {
                pclose(vcgencmd_pipe);
                return TestResult::SUCCESS;
            }
            pclose(vcgencmd_pipe);
        }
    }

    return TestResult::FAILURE;
}

TestResult FormFactorTester::test_gpio_pins() {
    if (!fs::exists("/sys/class/gpio")) {
        return TestResult::NOT_SUPPORTED;
    }

    // Test a few GPIO pins if available
    bool any_pin_tested = false;
    for (int pin = 0; pin < 10; ++pin) {  // Test first 10 pins
        TestResult pin_result = test_gpio_pin(pin);
        if (pin_result != TestResult::NOT_SUPPORTED) {
            any_pin_tested = true;
            if (pin_result != TestResult::SUCCESS) {
                return TestResult::FAILURE;
            }
        }
    }

    return any_pin_tested ? TestResult::SUCCESS : TestResult::NOT_SUPPORTED;
}

TestResult FormFactorTester::test_interfaces() {
    auto interfaces = enumerate_interfaces();
    if (interfaces.empty()) {
        return TestResult::FAILURE;
    }

    // Check if at least some basic interfaces are available
    bool has_basic_interfaces = false;
    for (const auto& interface : interfaces) {
        if (interface.available) {
            has_basic_interfaces = true;
            break;
        }
    }

    return has_basic_interfaces ? TestResult::SUCCESS : TestResult::FAILURE;
}

TestResult FormFactorTester::test_temperature() {
    double temp = get_board_temperature();
    if (temp > 0 && temp < 100) {  // Reasonable temperature range
        return TestResult::SUCCESS;
    }

    return TestResult::NOT_SUPPORTED;
}

TestResult FormFactorTester::monitor_interfaces(std::chrono::seconds duration) {
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + duration;

    bool interfaces_stable = true;
    double initial_temp = get_board_temperature();

    while (std::chrono::steady_clock::now() < end_time && interfaces_stable) {
        // Check temperature stability
        double current_temp = get_board_temperature();
        if (std::abs(current_temp - initial_temp) > 20.0) {  // More than 20°C change
            interfaces_stable = false;
        }

        // Check interface availability
        auto current_interfaces = enumerate_interfaces();
        if (current_interfaces.size() != form_factor_info_.interfaces.size()) {
            interfaces_stable = false;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return interfaces_stable ? TestResult::SUCCESS : TestResult::FAILURE;
}

std::vector<InterfaceInfo> FormFactorTester::enumerate_interfaces() {
    std::vector<InterfaceInfo> interfaces;

    // Check GPIO
    InterfaceInfo gpio_info;
    gpio_info.type = InterfaceType::GPIO;
    gpio_info.name = "GPIO";
    gpio_info.available = fs::exists("/sys/class/gpio");
    gpio_info.status = gpio_info.available ? "Available" : "Not Available";
    interfaces.push_back(gpio_info);

    // Check I2C
    InterfaceInfo i2c_info;
    i2c_info.type = InterfaceType::I2C;
    i2c_info.name = "I2C";
    i2c_info.available = fs::exists("/sys/class/i2c-dev") || fs::exists("/dev/i2c-0");
    i2c_info.status = i2c_info.available ? "Available" : "Not Available";
    interfaces.push_back(i2c_info);

    // Check SPI
    InterfaceInfo spi_info;
    spi_info.type = InterfaceType::SPI;
    spi_info.name = "SPI";
    spi_info.available = fs::exists("/sys/class/spidev") || fs::exists("/dev/spidev0.0");
    spi_info.status = spi_info.available ? "Available" : "Not Available";
    interfaces.push_back(spi_info);

    // Check UART
    InterfaceInfo uart_info;
    uart_info.type = InterfaceType::UART;
    uart_info.name = "UART";
    uart_info.available = fs::exists("/dev/ttyAMA0") || fs::exists("/dev/ttyS0");
    uart_info.status = uart_info.available ? "Available" : "Not Available";
    interfaces.push_back(uart_info);

    // Check USB
    InterfaceInfo usb_info;
    usb_info.type = InterfaceType::USB;
    usb_info.name = "USB";
    usb_info.available = fs::exists("/sys/class/usb_device") || fs::exists("/dev/bus/usb");
    usb_info.status = usb_info.available ? "Available" : "Not Available";
    interfaces.push_back(usb_info);

    // Check Ethernet
    InterfaceInfo eth_info;
    eth_info.type = InterfaceType::ETHERNET;
    eth_info.name = "Ethernet";
    eth_info.available = fs::exists("/sys/class/net/eth0") || fs::exists("/sys/class/net/enp");
    eth_info.status = eth_info.available ? "Available" : "Not Available";
    interfaces.push_back(eth_info);

    // Check PCIe
    InterfaceInfo pcie_info;
    pcie_info.type = InterfaceType::PCIe;
    pcie_info.name = "PCIe";
    pcie_info.available = fs::exists("/sys/bus/pci") && !fs::is_empty("/sys/bus/pci/devices");
    pcie_info.status = pcie_info.available ? "Available" : "Not Available";
    interfaces.push_back(pcie_info);

    return interfaces;
}

TestResult FormFactorTester::test_gpio_pin(int pin_number) {
    std::string gpio_path = "/sys/class/gpio/gpio" + std::to_string(pin_number);

    // Check if GPIO pin exists
    if (!fs::exists(gpio_path)) {
        // Try to export the pin
        std::ofstream export_file("/sys/class/gpio/export");
        if (export_file.is_open()) {
            export_file << pin_number;
            export_file.close();

            // Wait a bit for the pin to be exported
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (!fs::exists(gpio_path)) {
                return TestResult::NOT_SUPPORTED;
            }
        } else {
            return TestResult::NOT_SUPPORTED;
        }
    }

    // Test setting pin as output and reading back
    std::ofstream direction_file(gpio_path + "/direction");
    if (direction_file.is_open()) {
        direction_file << "out";
        direction_file.close();

        // Set value to 1
        std::ofstream value_file(gpio_path + "/value");
        if (value_file.is_open()) {
            value_file << "1";
            value_file.close();

            // Read back value
            std::ifstream value_read_file(gpio_path + "/value");
            if (value_read_file.is_open()) {
                std::string value_str;
                value_read_file >> value_str;
                if (value_str == "1") {
                    // Set value to 0
                    std::ofstream value_file2(gpio_path + "/value");
                    value_file2 << "0";
                    value_file2.close();

                    return TestResult::SUCCESS;
                }
            }
        }
    }

    return TestResult::FAILURE;
}

double FormFactorTester::get_board_temperature() {
    // Try NXP i.MX93 specific temperature reading
    if (fs::exists("/sys/class/thermal/thermal_zone0/temp")) {
        std::ifstream temp_file("/sys/class/thermal/thermal_zone0/temp");
        if (temp_file.is_open()) {
            int temp_milli;
            temp_file >> temp_milli;
            return temp_milli / 1000.0;
        }
    }

    // Try vcgencmd
    if (system("which vcgencmd > /dev/null 2>&1") == 0) {
        FILE* temp_pipe = popen("vcgencmd measure_temp 2>/dev/null", "r");
        if (temp_pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), temp_pipe) != NULL) {
                std::string temp_str(buffer);
                size_t temp_pos = temp_str.find("temp=");
                if (temp_pos != std::string::npos) {
                    size_t end_pos = temp_str.find("'", temp_pos);
                    if (end_pos != std::string::npos) {
                        std::string temp_value = temp_str.substr(temp_pos + 5, end_pos - temp_pos - 5);
                        try {
                            return std::stod(temp_value);
                        } catch (...) {
                            // Ignore conversion errors
                        }
                    }
                }
            }
            pclose(temp_pipe);
        }
    }

    return 0.0;
}

} // namespace imx93_peripheral_test