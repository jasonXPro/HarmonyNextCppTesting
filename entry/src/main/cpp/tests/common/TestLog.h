//
// Created on 2024/7/16.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef GTestHelper_TESTLOG_H
#define GTestHelper_TESTLOG_H
#undef LOG_DOMAIN
#define LOG_DOMAIN 0x3200 // 全局domain宏，标识业务领域
#include <hilog/log.h>
#include <format>
#include <string>
namespace GTestHelper {
static std::vector<char> formatString(const char *fmt, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args); // 复制args以便安全重用

    // 尝试使用初始大小的缓冲区进行格式化
    std::vector<char> buffer(150);
    int needed = std::vsnprintf(buffer.data(), buffer.size(), fmt, args);

    // 检查是否有足够的空间，如果没有，则调整大小并重试
    if (needed >= static_cast<int>(buffer.size())) {
        buffer.resize(needed + 1);
        std::vsnprintf(buffer.data(), buffer.size(), fmt, args_copy);
    }

    va_end(args_copy); // 清理复制的va_list

    // 创建并返回一个std::string对象
    return buffer;
}

static void LOG_ERROR(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    //
    const auto buffer = formatString(fmt, args);
    //
    OH_LOG_Print(LOG_APP, LogLevel::LOG_ERROR, LOG_DOMAIN, "GTestHelper", "%{public}s", buffer.data());

    va_end(args);
}

static void LOG_WARN(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    //
    const auto buffer = formatString(fmt, args);
    //
    OH_LOG_Print(LOG_APP, LogLevel::LOG_WARN, LOG_DOMAIN, "GTestHelper", "%{public}s", buffer.data());

    va_end(args);
}
static void LOG_INFO(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    //
    const auto buffer = formatString(fmt, args);
    //
    OH_LOG_Print(LOG_APP, LogLevel::LOG_INFO, LOG_DOMAIN, "GTestHelper", "%{public}s", buffer.data());

    va_end(args);
}
} // namespace GTestHelper
#endif // GTestHelper_TESTLOG_H
