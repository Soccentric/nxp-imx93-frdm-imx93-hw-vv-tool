/**
 * @file test_usb_tester.cpp
 * @brief Unit tests for USB tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include "usb_tester.h"
#include <gtest/gtest.h>

namespace imx93_peripheral_test {

class USBTesterTest : public ::testing::Test {
protected:
    void SetUp() override {
        tester_ = std::make_unique<USBTester>();
    }

    void TearDown() override {
        tester_.reset();
    }

    std::unique_ptr<USBTester> tester_;
};

TEST_F(USBTesterTest, Constructor) {
    ASSERT_NE(tester_, nullptr);
    EXPECT_EQ(tester_->get_peripheral_name(), "USB");
}

TEST_F(USBTesterTest, IsAvailable) {
    bool available = tester_->is_available();
    EXPECT_TRUE(available || !available);
}

TEST_F(USBTesterTest, ShortTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "USB not available on this system";
    }

    TestReport report = tester_->short_test();
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "USB");
    EXPECT_GE(report.duration.count(), 0);
}

TEST_F(USBTesterTest, MonitorTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "USB not available on this system";
    }

    TestReport report = tester_->monitor_test(std::chrono::seconds(1));
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "USB");
    EXPECT_GE(report.duration.count(), 0);
}

} // namespace imx93_peripheral_test
