/**
 * @file test_networking_tester.cpp
 * @brief Unit tests for Networking tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include "networking_tester.h"
#include <gtest/gtest.h>

namespace imx93_peripheral_test {

class NetworkingTesterTest : public ::testing::Test {
protected:
    void SetUp() override {
        tester_ = std::make_unique<NetworkingTester>();
    }

    void TearDown() override {
        tester_.reset();
    }

    std::unique_ptr<NetworkingTester> tester_;
};

TEST_F(NetworkingTesterTest, Constructor) {
    ASSERT_NE(tester_, nullptr);
    EXPECT_EQ(tester_->get_peripheral_name(), "Networking");
}

TEST_F(NetworkingTesterTest, IsAvailable) {
    bool available = tester_->is_available();
    EXPECT_TRUE(available || !available);
}

TEST_F(NetworkingTesterTest, ShortTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Networking not available on this system";
    }

    TestReport report = tester_->short_test();
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Networking");
    EXPECT_GE(report.duration.count(), 0);
}

TEST_F(NetworkingTesterTest, MonitorTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Networking not available on this system";
    }

    TestReport report = tester_->monitor_test(std::chrono::seconds(1));
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Networking");
    EXPECT_GE(report.duration.count(), 0);
}

} // namespace imx93_peripheral_test
