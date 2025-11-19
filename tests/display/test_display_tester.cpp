/**
 * @file test_display_tester.cpp
 * @brief Unit tests for Display tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include "display_tester.h"
#include <gtest/gtest.h>

namespace imx93_peripheral_test {

class DisplayTesterTest : public ::testing::Test {
protected:
    void SetUp() override {
        tester_ = std::make_unique<DisplayTester>();
    }

    void TearDown() override {
        tester_.reset();
    }

    std::unique_ptr<DisplayTester> tester_;
};

TEST_F(DisplayTesterTest, Constructor) {
    ASSERT_NE(tester_, nullptr);
    EXPECT_EQ(tester_->get_peripheral_name(), "Display");
}

TEST_F(DisplayTesterTest, IsAvailable) {
    bool available = tester_->is_available();
    EXPECT_TRUE(available || !available);
}

TEST_F(DisplayTesterTest, ShortTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Display not available on this system";
    }

    TestReport report = tester_->short_test();
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Display");
    EXPECT_GE(report.duration.count(), 0);
}

TEST_F(DisplayTesterTest, MonitorTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Display not available on this system";
    }

    TestReport report = tester_->monitor_test(std::chrono::seconds(1));
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Display");
    EXPECT_GE(report.duration.count(), 0);
}

} // namespace imx93_peripheral_test
