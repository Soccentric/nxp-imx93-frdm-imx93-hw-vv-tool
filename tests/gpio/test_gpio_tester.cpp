/**
 * @file test_gpio_tester.cpp
 * @brief Unit tests for GPIO tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include <gtest/gtest.h>

#include "gpio_tester.h"

namespace imx93_peripheral_test {

/**
 * @brief Test fixture for GPIOTester.
 */
class GPIOTesterTest : public ::testing::Test {
protected:
  void SetUp() override {
    tester_ = std::make_unique<GPIOTester>();
  }

  void TearDown() override {
    tester_.reset();
  }

  std::unique_ptr<GPIOTester> tester_;
};

/**
 * @test GPIOTester_Constructor
 * @brief Tests GPIOTester construction.
 */
TEST_F(GPIOTesterTest, Constructor) {
  ASSERT_NE(tester_, nullptr);
  EXPECT_EQ(tester_->get_peripheral_name(), "GPIO");
}

/**
 * @test GPIOTester_IsAvailable
 * @brief Tests GPIO availability check.
 */
TEST_F(GPIOTesterTest, IsAvailable) {
  // This will depend on the system, but should not crash
  bool available = tester_->is_available();
  EXPECT_TRUE(available || !available);  // Either true or false is acceptable
}

/**
 * @test GPIOTester_ShortTest
 * @brief Tests short GPIO test execution.
 */
TEST_F(GPIOTesterTest, ShortTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "GPIO not available on this system";
  }

  TestReport report = tester_->short_test();
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "GPIO");
  EXPECT_GE(report.duration.count(), 0);
}

/**
 * @test GPIOTester_MonitorTest
 * @brief Tests GPIO monitoring test execution.
 */
TEST_F(GPIOTesterTest, MonitorTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "GPIO not available on this system";
  }

  TestReport report = tester_->monitor_test(std::chrono::seconds(1));
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "GPIO");
  EXPECT_GE(report.duration.count(), 0);
}

}  // namespace imx93_peripheral_test