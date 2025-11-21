/**
 * @file test_camera_tester.cpp
 * @brief Unit tests for Camera tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include <gtest/gtest.h>

#include "camera_tester.h"

namespace imx93_peripheral_test {

class CameraTesterTest : public ::testing::Test {
protected:
  void SetUp() override {
    tester_ = std::make_unique<CameraTester>();
  }

  void TearDown() override {
    tester_.reset();
  }

  std::unique_ptr<CameraTester> tester_;
};

TEST_F(CameraTesterTest, Constructor) {
  ASSERT_NE(tester_, nullptr);
  EXPECT_EQ(tester_->get_peripheral_name(), "Camera");
}

TEST_F(CameraTesterTest, IsAvailable) {
  bool available = tester_->is_available();
  EXPECT_TRUE(available || !available);
}

TEST_F(CameraTesterTest, ShortTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "Camera not available on this system";
  }

  TestReport report = tester_->short_test();
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "Camera");
  EXPECT_GE(report.duration.count(), 0);
}

TEST_F(CameraTesterTest, MonitorTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "Camera not available on this system";
  }

  TestReport report = tester_->monitor_test(std::chrono::seconds(1));
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "Camera");
  EXPECT_GE(report.duration.count(), 0);
}

}  // namespace imx93_peripheral_test
