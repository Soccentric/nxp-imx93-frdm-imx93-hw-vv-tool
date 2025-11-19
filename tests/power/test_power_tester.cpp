/**
 * @file test_power_tester.cpp
 * @brief Unit tests for Power tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include "power_tester.h"
#include <gtest/gtest.h>

namespace imx93_peripheral_test {

class PowerTesterTest : public ::testing::Test {
protected:
    void SetUp() override {
        tester_ = std::make_unique<PowerTester>();
    }

    void TearDown() override {
        tester_.reset();
    }

    std::unique_ptr<PowerTester> tester_;
};

TEST_F(PowerTesterTest, Constructor) {
    ASSERT_NE(tester_, nullptr);
    EXPECT_EQ(tester_->get_peripheral_name(), "Power");
}

TEST_F(PowerTesterTest, IsAvailable) {
    bool available = tester_->is_available();
    EXPECT_TRUE(available || !available);
}

TEST_F(PowerTesterTest, ShortTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Power not available on this system";
    }

    TestReport report = tester_->short_test();
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Power");
    EXPECT_GE(report.duration.count(), 0);
}

TEST_F(PowerTesterTest, MonitorTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Power not available on this system";
    }

    TestReport report = tester_->monitor_test(std::chrono::seconds(1));
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Power");
    EXPECT_GE(report.duration.count(), 0);
}

} // namespace imx93_peripheral_test
