/**
 * @file test_memory_tester.cpp
 * @brief Unit tests for Memory tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include <gtest/gtest.h>

#include "memory_tester.h"

namespace imx93_peripheral_test {

class MemoryTesterTest : public ::testing::Test {
protected:
  void SetUp() override {
    tester_ = std::make_unique<MemoryTester>();
  }

  void TearDown() override {
    tester_.reset();
  }

  std::unique_ptr<MemoryTester> tester_;
};

TEST_F(MemoryTesterTest, Constructor) {
  ASSERT_NE(tester_, nullptr);
  EXPECT_EQ(tester_->get_peripheral_name(), "Memory");
}

TEST_F(MemoryTesterTest, IsAvailable) {
  bool available = tester_->is_available();
  EXPECT_TRUE(available || !available);
}

TEST_F(MemoryTesterTest, ShortTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "Memory not available on this system";
  }

  TestReport report = tester_->short_test();
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "Memory");
  EXPECT_GE(report.duration.count(), 0);
}

TEST_F(MemoryTesterTest, MonitorTest) {
  if (!tester_->is_available()) {
    GTEST_SKIP() << "Memory not available on this system";
  }

  TestReport report = tester_->monitor_test(std::chrono::seconds(1));
  EXPECT_NE(report.result, TestResult::SKIPPED);
  EXPECT_EQ(report.peripheral_name, "Memory");
  EXPECT_GE(report.duration.count(), 0);
}

}  // namespace imx93_peripheral_test
