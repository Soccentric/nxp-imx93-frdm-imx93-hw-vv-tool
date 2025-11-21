/**
 * @file camera_tester.h
 * @brief Camera peripheral tester for FRDM-IMX93 verification.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 *
 * This header defines the Camera tester class that implements comprehensive
 * testing and monitoring of camera functionality on the NXP FRDM-IMX93.
 */

#ifndef CAMERA_TESTER_H
#define CAMERA_TESTER_H

#include <string>
#include <vector>

#include "peripheral_tester.h"

namespace imx93_peripheral_test {

/**
 * @struct CameraInfo
 * @brief Structure containing camera information.
 */
struct CameraInfo {
  std::string device_path;
  std::string driver_name;
  std::string sensor_name;
  std::string resolution;
  int         fps;
  bool        connected;
  bool        streaming;
};

/**
 * @class CameraTester
 * @brief Tester implementation for camera peripherals.
 *
 * This class implements the PeripheralTester interface to provide
 * comprehensive testing of camera functionality including MIPI CSI-2
 * interfaces on the NXP FRDM-IMX93.
 */
class CameraTester : public PeripheralTester {
public:
  /**
   * @brief Constructs a Camera tester instance.
   */
  CameraTester();

  /**
   * @brief Performs short verification test of camera functionality.
   *
   * Tests basic camera operations including:
   * - Camera detection and enumeration
   * - Sensor communication
   * - Basic capture functionality
   *
   * @return TestReport with detailed results.
   */
  TestReport short_test() override;

  /**
   * @brief Performs extended monitoring of camera stability.
   *
   * Monitors camera over time for:
   * - Signal stability
   * - Frame rate consistency
   * - Sensor temperature
   *
   * @param duration Monitoring duration in seconds.
   * @return TestReport with monitoring results.
   */
  TestReport monitor_test(std::chrono::seconds duration) override;

  /**
   * @brief Returns the peripheral name.
   * @return "Camera" as the peripheral identifier.
   */
  std::string get_peripheral_name() const override {
    return "Camera";
  }

  /**
   * @brief Checks if camera interfaces are available on the system.
   * @return true if camera interfaces are accessible.
   */
  bool is_available() const override;

private:
  /**
   * @brief Enumerates all camera devices on the system.
   * @return Vector of CameraInfo structures.
   */
  std::vector<CameraInfo> enumerate_cameras();

  /**
   * @brief Tests MIPI CSI-2 camera functionality.
   * @return TestResult indicating success or failure.
   */
  TestResult test_mipi_csi2();

  /**
   * @brief Tests camera sensor communication.
   * @param camera Camera device to test.
   * @return TestResult indicating success or failure.
   */
  TestResult test_camera_sensor(const CameraInfo& camera);

  /**
   * @brief Tests camera capture functionality.
   * @param camera Camera device to test.
   * @return TestResult indicating success or failure.
   */
  TestResult test_camera_capture(const CameraInfo& camera);

  /**
   * @brief Tests camera resolution and frame rates.
   * @param camera Camera device to test.
   * @return TestResult indicating success or failure.
   */
  TestResult test_camera_resolution(const CameraInfo& camera);

  /**
   * @brief Monitors camera streaming stability over time.
   * @param duration Monitoring duration.
   * @return TestResult indicating success or failure.
   */
  TestResult monitor_camera_streaming(std::chrono::seconds duration);

  /**
   * @brief Tests multiple camera support (up to 2 CSI-2 ports).
   * @return TestResult indicating success or failure.
   */
  TestResult test_multi_camera();

  std::vector<CameraInfo> cameras_;
  bool                    camera_available_;
};

}  // namespace imx93_peripheral_test

#endif  // CAMERA_TESTER_H