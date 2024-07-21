//
// Created on 2024/7/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef GTestHelper_CUSTOMTESTLISTENER_H
#define GTestHelper_CUSTOMTESTLISTENER_H
#include <gtest/gtest.h>
#include "TestLog.h"
namespace GTestHelper {

class CustomTestListener : public testing::TestEventListener {
public:
    void OnTestProgramStart(const testing::UnitTest &unit_test) override {
        LOG_INFO("Starting test program...");
        testCaseIndex = 1;
        LOG_INFO("========================================================================");
    }

    void OnTestProgramEnd(const testing::UnitTest &unit_test) override {
        LOG_INFO("Test program ended:");
        LOG_INFO("Total tests run: %d", unit_test.total_test_count());
        LOG_INFO("Tests passed: %d", unit_test.successful_test_count());
        LOG_INFO("Tests failed: %d", unit_test.failed_test_count());
    }

    void OnTestStart(const testing::TestInfo &test_info) override {
        //         LOG_INFO("Running test: %s.%s", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestEnd(const testing::TestInfo &test_info) override {
        bool passed = test_info.result()->Passed();
        std::string message = passed ? "Test passed" : "Test failed";

        if (test_info.result()->total_part_count() > 0) {
            const testing::TestPartResult &part_result = test_info.result()->GetTestPartResult(0);
            LOG_ERROR("[%d-%d]Test failed: %s.%s\nsummary: %s", testCaseIndex, index, test_info.test_case_name(),
                      test_info.name(), part_result.summary());
        } else {
            LOG_INFO("[%d-%d]Test passed: %s.%s", testCaseIndex, index, test_info.test_case_name(), test_info.name());
        }
        index++;
        LOG_INFO("===================================");
    }

    void OnTestCaseStart(const testing::TestCase &test_case) override {
        LOG_WARN("Starting test case: [%d]%s", testCaseIndex, test_case.name());
        index = 1;
    }

    // Called after a test case ends.
    void OnTestCaseEnd(const testing::TestCase &test_case) override {
        LOG_WARN("Test case ended:[%d]%s", testCaseIndex, test_case.name());
        testCaseIndex++;
        LOG_INFO("========================================================================");
    }

    // Called before a test iteration starts.
    void OnTestIterationStart(const testing::UnitTest &unit_test, int iteration) override {
    }

    // Called after a test iteration ends.
    void OnTestIterationEnd(const testing::UnitTest &unit_test, int iteration) override {
    }

    // Implement the pure virtual methods
    void OnEnvironmentsSetUpStart(const testing::UnitTest &unit_test) override {
    }

    void OnEnvironmentsSetUpEnd(const testing::UnitTest &unit_test) override {
    }

    void OnTestPartResult(const testing::TestPartResult &test_part_result) override {
    }

    void OnEnvironmentsTearDownStart(const testing::UnitTest &unit_test) override {
    }

    void OnEnvironmentsTearDownEnd(const testing::UnitTest &unit_test) override {
    }

private:
    int index = 1;
    int testCaseIndex = 1;
};
} // namespace GTestHelper
#endif // GTestHelper_CUSTOMTESTLISTENER_H
