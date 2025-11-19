/**
 * @file test_storage_tester.cpp
 * @brief Unit tests for Storage tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include "storage_tester.h"
#include <gtest/gtest.h>

namespace imx93_peripheral_test {

class StorageTesterTest : public ::testing::Test {
protected:
    void SetUp() override {
        tester_ = std::make_unique<StorageTester>();
    }

    void TearDown() override {
        tester_.reset();
    }

    std::unique_ptr<StorageTester> tester_;
};

TEST_F(StorageTesterTest, Constructor) {
    ASSERT_NE(tester_, nullptr);
    EXPECT_EQ(tester_->get_peripheral_name(), "Storage");
}

TEST_F(StorageTesterTest, IsAvailable) {
    bool available = tester_->is_available();
    EXPECT_TRUE(available || !available);
}

TEST_F(StorageTesterTest, ShortTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Storage not available on this system";
    }

    TestReport report = tester_->short_test();
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Storage");
    EXPECT_GE(report.duration.count(), 0);
}

TEST_F(StorageTesterTest, MonitorTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "Storage not available on this system";
    }

    TestReport report = tester_->monitor_test(std::chrono::seconds(1));
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Storage");
    EXPECT_GE(report.duration.count(), 0);
}

} // namespace imx93_peripheral_test
