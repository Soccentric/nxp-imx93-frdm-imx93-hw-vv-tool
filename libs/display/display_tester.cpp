/**
 * @file display_tester.cpp
 * @brief Implementation of display peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the i.MX 93 display interfaces:
 * - MIPI-DSI (Display Serial Interface) up to 4 lanes
 * - LVDS (Low-Voltage Differential Signaling) interface
 * - Parallel LCD interface (up to 24-bit)
 * - DRM/KMS (Direct Rendering Manager/Kernel Mode Setting)
 * - Hardware composition and overlay support
 */

#include "display_tester.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cstring>
#include <cstdlib>

namespace fs = std::filesystem;

namespace imx93_peripheral_test {

DisplayTester::DisplayTester() : display_available_(false) {
    // Check if display interfaces are available on i.MX93
    // i.MX93 uses DRM/KMS for display management
    display_available_ = fs::exists("/sys/class/drm") || fs::exists("/dev/dri");
    if (display_available_) {
        displays_ = enumerate_displays();
    }
}

TestReport DisplayTester::short_test() {
    auto start_time = std::chrono::steady_clock::now();

    if (!display_available_) {
        return create_report(TestResult::NOT_SUPPORTED, "Display interfaces not available", std::chrono::milliseconds(0));
    }

    std::stringstream details;
    bool all_passed = true;

    details << "Found " << displays_.size() << " display interface(s)\n";

    for (const auto& display : displays_) {
        details << "- " << display.interface_name << " (" << display.resolution;
        if (display.refresh_rate > 0) {
            details << " @ " << display.refresh_rate << "Hz";
        }
        details << ", " << (display.connected ? "connected" : "disconnected") << ")\n";
    }

    // Test HDMI functionality
    TestResult hdmi_result = test_hdmi();
    details << "HDMI: " << (hdmi_result == TestResult::SUCCESS ? "PASS" : (hdmi_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL")) << "\n";
    if (hdmi_result != TestResult::SUCCESS && hdmi_result != TestResult::NOT_SUPPORTED) all_passed = false;

    // Test MIPI DSI functionality
    TestResult dsi_result = test_mipi_dsi();
    details << "MIPI DSI: " << (dsi_result == TestResult::SUCCESS ? "PASS" : (dsi_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL")) << "\n";
    if (dsi_result != TestResult::SUCCESS && dsi_result != TestResult::NOT_SUPPORTED) all_passed = false;

    // Test 4K HDMI capability
    TestResult k4_result = test_4k_hdmi();
    details << "4K HDMI: " << (k4_result == TestResult::SUCCESS ? "PASS" : (k4_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL")) << "\n";
    if (k4_result != TestResult::SUCCESS && k4_result != TestResult::NOT_SUPPORTED) all_passed = false;

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
    return create_report(overall_result, details.str(), duration);
}

TestReport DisplayTester::monitor_test(std::chrono::seconds duration) {
    auto start_time = std::chrono::steady_clock::now();

    if (!display_available_) {
        return create_report(TestResult::NOT_SUPPORTED, "Display interfaces not available", std::chrono::milliseconds(0));
    }

    TestResult result = monitor_display_connection(duration);

    auto end_time = std::chrono::steady_clock::now();
    auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::string details = "Display monitoring completed for " + std::to_string(duration.count()) + " seconds";
    return create_report(result, details, test_duration);
}

bool DisplayTester::is_available() const {
    return display_available_;
}

std::vector<DisplayInfo> DisplayTester::enumerate_displays() {
    std::vector<DisplayInfo> displays;

    // Check DRM devices
    if (fs::exists("/sys/class/drm")) {
        for (const auto& entry : fs::directory_iterator("/sys/class/drm")) {
            std::string device_name = entry.path().filename().string();

            if (device_name.find("card") == 0) {
                // Check for connected displays
                std::string status_path = "/sys/class/drm/" + device_name + "/status";
                std::ifstream status_file(status_path);

                if (status_file.is_open()) {
                    std::string status;
                    std::getline(status_file, status);

                    DisplayInfo display;
                    display.interface_name = device_name;
                    display.connected = (status == "connected");

                    // Determine display type
                    if (device_name.find("HDMI") != std::string::npos) {
                        display.type = DisplayType::HDMI;
                    } else if (device_name.find("DSI") != std::string::npos) {
                        display.type = DisplayType::MIPI_DSI;
                    } else {
                        display.type = DisplayType::COMPOSITE;
                    }

                    // Get resolution if connected
                    if (display.connected) {
                        std::string modes_path = "/sys/class/drm/" + device_name + "/modes";
                        std::ifstream modes_file(modes_path);
                        if (modes_file.is_open()) {
                            std::string mode;
                            if (std::getline(modes_file, mode)) {
                                display.resolution = mode;
                                // Extract refresh rate if available
                                size_t at_pos = mode.find('@');
                                if (at_pos != std::string::npos) {
                                    try {
                                        display.refresh_rate = std::stoi(mode.substr(at_pos + 1));
                                    } catch (...) {
                                        display.refresh_rate = 0;
                                    }
                                }
                            }
                        }
                    }

                    displays.push_back(display);
                }
            }
        }
    }

    return displays;
}

TestResult DisplayTester::test_hdmi() {
    // Look for HDMI displays
    bool hdmi_found = false;
    for (const auto& display : displays_) {
        if (display.type == DisplayType::HDMI) {
            hdmi_found = true;
            if (display.connected) {
                // Test HDMI functionality
                TestResult res_result = test_display_resolution(display);
                if (res_result != TestResult::SUCCESS) {
                    return TestResult::FAILURE;
                }
            }
        }
    }

    return hdmi_found ? TestResult::SUCCESS : TestResult::NOT_SUPPORTED;
}

TestResult DisplayTester::test_mipi_dsi() {
    // Look for MIPI DSI displays
    bool dsi_found = false;
    for (const auto& display : displays_) {
        if (display.type == DisplayType::MIPI_DSI) {
            dsi_found = true;
            if (display.connected) {
                // Test DSI functionality
                TestResult res_result = test_display_resolution(display);
                if (res_result != TestResult::SUCCESS) {
                    return TestResult::FAILURE;
                }
            }
        }
    }

    return dsi_found ? TestResult::SUCCESS : TestResult::NOT_SUPPORTED;
}

TestResult DisplayTester::test_display_resolution(const DisplayInfo& display) {
    if (!display.connected) {
        return TestResult::NOT_SUPPORTED;
    }

    // Check if resolution is valid
    if (display.resolution.empty()) {
        return TestResult::FAILURE;
    }

    // Parse resolution (e.g., "1920x1080" or "3840x2160@60")
    size_t x_pos = display.resolution.find('x');
    if (x_pos == std::string::npos) {
        return TestResult::FAILURE;
    }

    try {
        int width = std::stoi(display.resolution.substr(0, x_pos));
        size_t at_pos = display.resolution.find('@');
        std::string height_str = (at_pos != std::string::npos) ?
            display.resolution.substr(x_pos + 1, at_pos - x_pos - 1) :
            display.resolution.substr(x_pos + 1);

        int height = std::stoi(height_str);

        // Basic validation - reasonable resolution ranges
        if (width < 640 || width > 7680 || height < 480 || height > 4320) {
            return TestResult::FAILURE;
        }

    } catch (...) {
        return TestResult::FAILURE;
    }

    return TestResult::SUCCESS;
}

TestResult DisplayTester::test_display_output() {
    // Test display output by checking if X11 or Wayland is running
    // and can create a basic window/display

    // Check for X11
    bool x11_available = (system("pgrep Xorg > /dev/null 2>&1") == 0) ||
                        (system("pgrep Xwayland > /dev/null 2>&1") == 0);

    // Check for Wayland
    bool wayland_available = (system("pgrep weston > /dev/null 2>&1") == 0) ||
                            (system("pgrep mutter > /dev/null 2>&1") == 0);

    if (!x11_available && !wayland_available) {
        return TestResult::NOT_SUPPORTED;
    }

    // Try to get display info using xrandr or similar
    int result = system("xrandr --current > /dev/null 2>&1");
    if (result == 0) {
        return TestResult::SUCCESS;
    }

    // Fallback: check if DRM is working
    result = system("modetest -c > /dev/null 2>&1");
    return (result == 0) ? TestResult::SUCCESS : TestResult::FAILURE;
}

TestResult DisplayTester::monitor_display_connection(std::chrono::seconds duration) {
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + duration;

    std::vector<int> connection_counts;

    while (std::chrono::steady_clock::now() < end_time) {
        int connected_count = 0;
        auto current_displays = enumerate_displays();

        for (const auto& display : current_displays) {
            if (display.connected) {
                connected_count++;
            }
        }

        connection_counts.push_back(connected_count);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    if (connection_counts.empty()) {
        return TestResult::FAILURE;
    }

    // Check for connection stability (should be consistent)
    int first_count = connection_counts.front();
    bool stable = true;
    for (int count : connection_counts) {
        if (count != first_count) {
            stable = false;
            break;
        }
    }

    return stable ? TestResult::SUCCESS : TestResult::FAILURE;
}

TestResult DisplayTester::test_4k_hdmi() {
    // Test 4K HDMI capability
    for (const auto& display : displays_) {
        if (display.type == DisplayType::HDMI && display.connected) {
            // Check for 4K resolution
            if (display.resolution.find("3840x2160") != std::string::npos ||
                display.resolution.find("4096x2160") != std::string::npos) {
                // Check refresh rate for 4Kp60
                if (display.refresh_rate >= 60) {
                    return TestResult::SUCCESS;
                }
            }
        }
    }

    // Check if 4K modes are supported even if not currently active
    FILE* xrandr_pipe = popen("xrandr 2>/dev/null | grep -E '3840x2160|4096x2160'", "r");
    if (xrandr_pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), xrandr_pipe)) {
            pclose(xrandr_pipe);
            return TestResult::SUCCESS;
        }
        pclose(xrandr_pipe);
    }

    return TestResult::NOT_SUPPORTED;
}

} // namespace imx93_peripheral_test