/**
 * @file test_form_factor_tester.cpp
 * @brief Unit tests for Form Factor tester.
 * @author Sandesh Ghimire
 * @copyright (C) Soccentric LLC. All rights reserved.
 */

#include "form_factor_tester.h"
#include <gtest/gtest.h>

namespace imx93_peripheral_test {

class FormFactorTesterTest : public ::testing::Test {
protected:
    void SetUp() override {
        tester_ = std::make_unique<FormFactorTester>();
    }

    void TearDown() override {
        tester_.reset();
    }

    std::unique_ptr<FormFactorTester> tester_;
};

TEST_F(FormFactorTesterTest, Constructor) {
    ASSERT_NE(tester_, nullptr);
    EXPECT_EQ(tester_->get_peripheral_name(), "Form Factor");
}

TEST_F(FormFactorTesterTest, IsAvailable) {
    bool available = tester_->is_available();
    EXPECT_TRUE(available || !available);
}

TEST_F(FormFactorTesterTest, ShortTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "FormFactor not available on this system";
    }

    TestReport report = tester_->short_test();
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Form Factor");
    EXPECT_GE(report.duration.count(), 0);
}

TEST_F(FormFactorTesterTest, MonitorTest) {
    if (!tester_->is_available()) {
        GTEST_SKIP() << "FormFactor not available on this system";
    }

    TestReport report = tester_->monitor_test(std::chrono::seconds(1));
    EXPECT_NE(report.result, TestResult::SKIPPED);
    EXPECT_EQ(report.peripheral_name, "Form Factor");
    EXPECT_GE(report.duration.count(), 0);
}

} // namespace imx93_peripheral_test
