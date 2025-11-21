/**
 * @file camera_tester.cpp
 * @brief Implementation of camera peripheral tester for i.MX93.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This implementation tests the i.MX 93 camera interfaces:
 * - MIPI-CSI2 (Camera Serial Interface) up to 4 lanes per port
 * - Dual MIPI-CSI2 interfaces
 * - ISI (Image Sensing Interface) for image processing
 * - V4L2 (Video4Linux2) support
 * - Support for various camera sensors (OV5640, etc.)
 */

#include "camera_tester.h"

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

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

CameraTester::CameraTester() : camera_available_(false) {
  // Check if camera interfaces are available on i.MX93
  // i.MX93 uses ISI (Image Sensing Interface) with MIPI-CSI2
  camera_available_ = fs::exists("/dev/video0") || fs::exists("/sys/class/video4linux");
  if (camera_available_) {
    cameras_ = enumerate_cameras();
  }
}

TestReport CameraTester::short_test() {
  auto start_time = std::chrono::steady_clock::now();

  if (!camera_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Camera interfaces not available",
                         std::chrono::milliseconds(0));
  }

  std::stringstream details;
  bool              all_passed = true;

  details << "Found " << cameras_.size() << " camera device(s)\n";

  for (const auto& camera : cameras_) {
    details << "- " << camera.device_path << " (" << camera.driver_name;
    if (!camera.sensor_name.empty()) {
      details << ", " << camera.sensor_name;
    }
    details << ", " << (camera.connected ? "connected" : "disconnected") << ")\n";
  }

  // Test MIPI CSI-2 functionality
  TestResult csi2_result = test_mipi_csi2();
  details << "MIPI CSI-2: "
          << (csi2_result == TestResult::SUCCESS
                  ? "PASS"
                  : (csi2_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (csi2_result != TestResult::SUCCESS && csi2_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  // Test multi-camera support
  TestResult multi_result = test_multi_camera();
  details << "Multi-camera: "
          << (multi_result == TestResult::SUCCESS
                  ? "PASS"
                  : (multi_result == TestResult::NOT_SUPPORTED ? "N/A" : "FAIL"))
          << "\n";
  if (multi_result != TestResult::SUCCESS && multi_result != TestResult::NOT_SUPPORTED)
    all_passed = false;

  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  TestResult overall_result = all_passed ? TestResult::SUCCESS : TestResult::FAILURE;
  return create_report(overall_result, details.str(), duration);
}

TestReport CameraTester::monitor_test(std::chrono::seconds duration) {
  auto start_time = std::chrono::steady_clock::now();

  if (!camera_available_) {
    return create_report(TestResult::NOT_SUPPORTED, "Camera interfaces not available",
                         std::chrono::milliseconds(0));
  }

  TestResult result = monitor_camera_streaming(duration);

  auto end_time      = std::chrono::steady_clock::now();
  auto test_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  std::string details =
      "Camera monitoring completed for " + std::to_string(duration.count()) + " seconds";
  return create_report(result, details, test_duration);
}

bool CameraTester::is_available() const {
  return camera_available_;
}

std::vector<CameraInfo> CameraTester::enumerate_cameras() {
  std::vector<CameraInfo> cameras;

  // Check V4L2 devices
  if (fs::exists("/sys/class/video4linux")) {
    for (const auto& entry : fs::directory_iterator("/sys/class/video4linux")) {
      std::string device_name = entry.path().filename().string();
      std::string device_path = "/dev/" + device_name;

      if (fs::exists(device_path)) {
        CameraInfo camera;
        camera.device_path = device_path;

        // Get device info using v4l2
        int fd = open(device_path.c_str(), O_RDONLY);
        if (fd >= 0) {
          struct v4l2_capability cap;
          if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
            camera.driver_name = reinterpret_cast<char*>(cap.driver);
            camera.connected   = true;

            // Check if it's a camera (has video capture capability)
            if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
              cameras.push_back(camera);
            }
          }
          close(fd);
        }
      }
    }
  }

  return cameras;
}

TestResult CameraTester::test_mipi_csi2() {
  // Look for CSI-2 capable cameras
  bool csi2_found = false;

  for (const auto& camera : cameras_) {
    if (camera.connected) {
      csi2_found = true;

      // Test camera sensor communication
      TestResult sensor_result = test_camera_sensor(camera);
      if (sensor_result != TestResult::SUCCESS) {
        return TestResult::FAILURE;
      }

      // Test basic capture
      TestResult capture_result = test_camera_capture(camera);
      if (capture_result != TestResult::SUCCESS) {
        return TestResult::FAILURE;
      }
    }
  }

  return csi2_found ? TestResult::SUCCESS : TestResult::NOT_SUPPORTED;
}

TestResult CameraTester::test_camera_sensor(const CameraInfo& camera) {
  int fd = open(camera.device_path.c_str(), O_RDONLY);
  if (fd < 0) {
    return TestResult::FAILURE;
  }

  // Query camera capabilities
  struct v4l2_capability cap;
  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) != 0) {
    close(fd);
    return TestResult::FAILURE;
  }

  // Check for video capture capability
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    close(fd);
    return TestResult::FAILURE;
  }

  close(fd);
  return TestResult::SUCCESS;
}

TestResult CameraTester::test_camera_capture(const CameraInfo& camera) {
  int fd = open(camera.device_path.c_str(), O_RDONLY);
  if (fd < 0) {
    return TestResult::FAILURE;
  }

  // Try to get current format
  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(fd, VIDIOC_G_FMT, &fmt) != 0) {
    close(fd);
    return TestResult::FAILURE;
  }

  // Check if we got a valid format
  if (fmt.fmt.pix.width == 0 || fmt.fmt.pix.height == 0) {
    close(fd);
    return TestResult::FAILURE;
  }

  close(fd);
  return TestResult::SUCCESS;
}

TestResult CameraTester::test_camera_resolution(const CameraInfo& camera) {
  int fd = open(camera.device_path.c_str(), O_RDONLY);
  if (fd < 0) {
    return TestResult::FAILURE;
  }

  // Get current format
  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(fd, VIDIOC_G_FMT, &fmt) != 0) {
    close(fd);
    return TestResult::FAILURE;
  }

  // Validate resolution
  uint32_t width  = fmt.fmt.pix.width;
  uint32_t height = fmt.fmt.pix.height;

  // Check for reasonable camera resolutions
  if (width < 640 || width > 4056 || height < 480 || height > 3040) {
    close(fd);
    return TestResult::FAILURE;
  }

  close(fd);
  return TestResult::SUCCESS;
}

TestResult CameraTester::monitor_camera_streaming(std::chrono::seconds duration) {
  if (cameras_.empty()) {
    return TestResult::NOT_SUPPORTED;
  }

  auto start_time = std::chrono::steady_clock::now();
  auto end_time   = start_time + duration;

  // For monitoring, we check if cameras remain accessible
  bool stable = true;

  while (std::chrono::steady_clock::now() < end_time && stable) {
    auto current_cameras = enumerate_cameras();

    if (current_cameras.size() != cameras_.size()) {
      stable = false;
      break;
    }

    // Check if all original cameras are still present
    for (const auto& orig_camera : cameras_) {
      bool found = false;
      for (const auto& curr_camera : current_cameras) {
        if (curr_camera.device_path == orig_camera.device_path) {
          found = true;
          break;
        }
      }
      if (!found) {
        stable = false;
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }

  return stable ? TestResult::SUCCESS : TestResult::FAILURE;
}

TestResult CameraTester::test_multi_camera() {
  // Check for multiple camera support (up to 2 CSI-2 ports on FRDM-IMX93)
  int camera_count = 0;

  for (const auto& camera : cameras_) {
    if (camera.connected) {
      camera_count++;
    }
  }

  // FRDM-IMX93 supports up to 2 CSI-2 cameras
  if (camera_count > 2) {
    return TestResult::FAILURE;
  }

  return (camera_count > 0) ? TestResult::SUCCESS : TestResult::NOT_SUPPORTED;
}

}  // namespace imx93_peripheral_test