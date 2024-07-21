//
// Created on 2024/7/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#ifndef GTestHelper_TESTHELPER_H
#define GTestHelper_TESTHELPER_H
#include <gtest/gtest.h>
#include "CustomTestListener.h"
namespace GTestHelper {
extern "C" {
int __llvm_profile_dump(void);
}
static void RunAllTests() {
    ::testing::InitGoogleTest();
    ::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
    CustomTestListener *listener = new CustomTestListener();
    listeners.Append(listener);

    RUN_ALL_TESTS();

    delete listener;
    
    //写入覆盖率profile数据
    int result = __llvm_profile_dump();
    if (result == 0) {
        LOG_WARN("Profile data dumped successfully.");
    } else {
        LOG_WARN("Failed to dump profile data.");
    }
}
} // namespace GTestHelper
#endif // GTestHelper_TESTHELPER_H
