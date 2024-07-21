//
// Created on 2024/7/20.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".
#include "gtest/gtest.h"
#include <my_math.h>

class MyMathTest : public ::testing::Test {
protected:
    // 测试用例的设置代码
    MyMath math;
    void SetUp() override {
        // 这里可以进行一些测试前的准备工作
    }

    // 测试用例的清理代码
    void TearDown() override {
        // 这里可以进行一些测试后的清理工作
    }
};

// 测试Add函数的加法功能
TEST_F(MyMathTest, TestAdd) {
    EXPECT_EQ(math.Add(1, 2), 3);
    EXPECT_EQ(math.Add(-1, 1), 0);
    EXPECT_EQ(math.Add(-1, -1), -2);
}