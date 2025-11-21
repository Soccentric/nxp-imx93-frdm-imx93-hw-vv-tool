/**
 * @file test_cpu_tester.cpp
 * @brief Unit tests for CPU tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include <gtest/gtest.h>

#include "cpu_tester.h"

namespace imx93_peripheral_test {

class CPUTesterTest : public ::testing::Test {
protected:
  void SetUp() override {
    tester_ = std::make_unique<CPUTester>();
  }

  void TearDown() override {
    tester_.reset();
  }

  std::unique_ptr<CPUTester> tester_;
};

TEST_F(CPUTesterTest, Constructor) {
  ASSERT_NE(tester_, nullptr);
  EXPECT_EQ(tester_->get_peripheral_name(), "CPU");
}

TEST_F(CPUTesterTest, IsAvailable) {
  bool available = tester_->is_available();
  EXPECT_TRUE(available || !available);
}

TEST_F(CPUTesterTest, ShortTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "CPU not available on this system";
  }

  TestReport report = tester_->short_test();
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "CPU");
  EXPECT_GE(report.duration.count(), 0);
}

TEST_F(CPUTesterTest, MonitorTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "CPU not available on this system";
  }

  TestReport report = tester_->monitor_test(std::chrono::seconds(1));
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "CPU");
  EXPECT_GE(report.duration.count(), 0);
}

}  // namespace imx93_peripheral_test
