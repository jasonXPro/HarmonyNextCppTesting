# 鸿蒙 NEXT C++ 组件单元测试及覆盖率统计方案探究

# 一、背景

鸿蒙NEXT操作系统中，上层的 ArkUI最终都会通过底层的 c++组件实现。而且随着鸿蒙的兴起，各大厂商纷纷拿出了各自的跨端方案，而c++组件复用是其中的一个主流方向。

为了确保这些组件的稳定性和可靠性，进行单元测试是其中很重要的一环，而目前鸿蒙的官方文档和开发工具中关于单元测试介绍更多是针对 ArkTs，c++单测的相关资料，因此本文尝试探究鸿蒙 NEXT c++组件单元测试及覆盖率统计的可行性方案。

# 二、单测框架的选择

在众多的 C++ 单元测试框架中，我们选择了 Google Test。它是一个成熟且功能强大的测试框架，广泛用于 C++ 项目的测试。

本文不会详细介绍Google Test的使用方法，如果你对 Google Test 感兴趣，可以访问[官方文档](https://google.github.io/googletest/)了解更多详情。

# 三、单元测试接入

本文将介绍两种在鸿蒙系统中接入单元测试的方案,各有优缺点:

1. so库直接接入,通过应用触发测试
2. 编译为可执行文件,通过命令行触发测试

## 方案一：so库接入单测，应用运行时触发

### 优势与缺点：

**优势：**

- 更加接近真实运行环境，减少了模拟（mock）的工作量。
- 可以在真机上运行测试。

**缺点：**

- 自动化收集测试结果较为繁琐。

### 核心步骤：

1. **配置 CMake**：启用测试并添加 Google Test 依赖。

```makefile
    enable_testing()

    # 使用 FetchContent 来下载和添加 Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/4b21f1abddaf7d28388d7430bab69a81eccb1909.zip
    )
    FetchContent_MakeAvailable(googletest)

    # 查找测试文件
    file(GLOB TESTSF
        tests/*.cpp
        tests/**/*.h
    )
```
2. **配置 CMake**：引入测试依赖。

```makefile
add_library(entry SHARED ${SOURCE_CORE} ${TESTS_CODE})
target_link_libraries(entry PUBLIC libace_napi.z.so libhilog_ndk.z.so gtest_main gmock_main)
#配置断言失效
target_compile_definitions(entry PRIVATE NDEBUG)
```

3. **手动触发测试**：应用运行时在调用 Google Test API。

```cpp
#include <gtest/gtest.h>

::testing::InitGoogleTest();
::testing::TestEventListeners &listeners = ::testing::UnitTest::GetInstance()->listeners();
CustomTestListener *listener = new CustomTestListener();
listeners.Append(listener);
RUN_ALL_TESTS();
delete listener;
```

4. **配置监听器**：实现自定义监听器以打印测试结果。

可以根据各自的需求，设置不同监听，打印所需日志

```cpp
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
```

### 效果

![gtest_hilog](https://github.com/jasonXPro/HarmonyNextCppTesting/blob/main/docs/gtest_hilog.png)

## 方案二：编译为可执行文件，命令行运行

### 优势与缺点：

**优势**：

- 便于自动化收集，便于配合流水线运行

**缺点**：

- 部分方法可能无法调用（例如网络请求），需要大量模拟（mock）。
- 无法在真机上运行。

### 核心步骤：

1. **配置 CMake**：启用测试并添加 Google Test 依赖。

```makefile
# 启用测试
enable_testing()
# 使用 FetchContent 来下载和添加 Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/4b21f1abddaf7d28388d7430bab69a81eccb1909.zip
)
FetchContent_MakeAvailable(googletest)
# 添加测试文件
file(GLOB TESTS_CODE
    tests/*.cpp
    tests/**/*.h
)
```

2. **配置 CMake**：配置可执行文件，引入google test依赖

```makefile
# 创建一个测试可执行文件
add_executable(entry_test ${SOURCE_CORE} ${TESTS_CODE})
# 链接 Google Test 库
target_link_libraries(entry_test libace_napi.z.so libhilog_ndk.z.so gtest_main gmock_main)
#配置断言失效
target_compile_definitions(entry_test PRIVATE NDEBUG)
```

3. **编译可执行文件**

注意：此时需要使用鸿蒙NDK提供的CMake编译,macOS路径如下：
`/Applications/DevEco-Studio.app/Contents/sdk/HarmonyOS-NEXT-DB1/openharmony/native/build-tools/cmake/bin/cmake` 

且需要配置CMAKE_SYSROOT、CMAKE_TOOLCHAIN_FILE等相关参数，参数含义见[官方文档](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/build-with-ndk-overview-0000001820880425)，示例如下：

```bash
#进入到makefile.txt同级目录

mkdir build && cd build
# 运行 cmake 命令 
/Applications/DevEco-Studio.app/Contents/sdk/HarmonyOS-NEXT-DB1/openharmony/native/build-tools/cmake/bin/cmake -DOHOS_STL=c++_shared -DCMAKE_SYSROOT=/Applications/DevEco-Studio.app/Contents/sdk/HarmonyOS-NEXT-DB1/hms/native/sysroot -DOHOS_PLATFORM=OHOS -DOHOS_ARCH=arm64-v8a -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=/Applications/DevEco-Studio.app/Contents/sdk/HarmonyOS-NEXT-DB1/hms/native/build/cmake/hmos.toolchain.cmake ..
# 构建项目
/Applications/DevEco-Studio.app/Contents/sdk/HarmonyOS-NEXT-DB1/openharmony/native/build-tools/cmake/bin/cmake  --build .
```

4. **发送到模拟器运行**：将可执行文件发送到模拟器并运行

> 根据测试，方案二仅能在模拟器中运行，真机会提示权限不足
> 

```bash
#EXECUTABLE_FILE_PATH为生成的可执行文件目录，即步骤 2 中的 project_test。示例路径为build/project_test
hdc file send ${EXECUTABLE_FILE_PATH} /data/local/tmp/

# 设置可执行权限，EXECUTABLE_FILE_NAME为可执行文件名称，示例为project_test
hdc shell chmod +x /data/local/tmp/${EXECUTABLE_FILE_NAME}
hdc shell "export LD_LIBRARY_PATH=/data/local/tmp; /data/local/tmp/${EXECUTABLE_FILE_NAME}"
```

### 效果：

![cmd_log](https://github.com/jasonXPro/HarmonyNextCppTesting/blob/main/docs/cmd_log.png)

# 四、生成单测覆盖率

要实现单测覆盖率生成，我们需要在上文单元测试接入的基础上添加覆盖率文件生成的相关配置。

## 核心步骤

1. **修改配置 CMake**：添加覆盖率相关配置

注意因为鸿蒙系统的读写权限，需要指定覆盖率数据文件的路径，且因方案一、二运行方式不同，需要指定不同的生成路径：

方案一：`-fprofile-instr-generate=/data/storage/el2/base/haps/entry/cache/default.profraw`

方案二： `-fprofile-instr-generate=/data/local/tmp/default.profraw`

具体示例：

方案一：

```makefile
add_library(entry SHARED ${SOURCE_CORE} ${TESTS_CODE})
target_link_libraries(entry PUBLIC libace_napi.z.so libhilog_ndk.z.so gtest_main gmock_main)
#配置断言失效
target_compile_definitions(entry PRIVATE NDEBUG)
#单测覆盖率配置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate=/data/storage/el2/base/haps/entry/cache/default.profraw -fcoverage-mapping -Wno-deprecated-declarations")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
```

方案二：

```bash
# 创建一个测试可执行文件
add_executable(entry_test ${SOURCE_CORE} ${TESTS_CODE})
# 链接 Google Test 库
target_link_libraries(entry_test libace_napi.z.so libhilog_ndk.z.so gtest_main gmock_main)
#配置断言失效
target_compile_definitions(entry_test PRIVATE NDEBUG)
#单测覆盖率配置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate=/data/local/tmp/default.profraw -fcoverage-mapping -Wno-deprecated-declarations")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
```

2. 运行方案一或方案二的单测后，调用hdc命令及llvm-cov命令生成覆盖率数据

```bash
# 添加覆盖率获取脚本
rm -rf default.profraw
hdc hdc file recv "/data/local/tmp/default.profraw" .
#获取覆盖率及html报告
rm -rf default.profdata
xcrun llvm-profdata merge -sparse default.profraw -o default.profdata
rm -rf coverage.html
xcrun llvm-cov show ./build/jdimage_test -instr-profile=./default.profdata -format=html > coverage.html
xcrun llvm-cov report ./build/jdimage_test -instr-profile=default.profdata
```

### 结果

![coverage](https://github.com/jasonXPro/HarmonyNextCppTesting/blob/main/docs/coverage.png)

![html](https://github.com/jasonXPro/HarmonyNextCppTesting/blob/main/docs/html.png)

# 五：总结

以上就是本文对鸿蒙 next c++组件的单测及覆盖率生成方案的探究，如果有错误或者更好的方式欢迎斧正。

相关demo已上传至 github，部分细节内容大家可以参考demo实现。

最后附上方案一及方案二的 shell脚本，大家可按需自行修改：
**方案一：**

```bash
#!/bin/bash

set -e  # 遇到错误立即退出
set -u  # 使用未定义的变量时报错

# 定义常量
PROFRAW="default.profraw"
PROFDATA="default.profdata"
COVERAGE_HTML="coverage.html"
REMOTE_PATH="/data/app/el2/100/base/com.example.hmgoogletestdemo/haps/entry/cache/${PROFRAW}"
EXE_DIR="../../../../build/default/intermediates/libs/default/arm64-v8a"
EXE_PATH="${EXE_DIR}/libentry.so"

# 清理旧文件
rm -rf "${PROFRAW}" "${PROFDATA}" "${COVERAGE_HTML}"

# 接收文件
hdc file recv "${REMOTE_PATH}" .

# 生成覆盖率数据
xcrun llvm-profdata merge -sparse "${PROFRAW}" -o "${PROFDATA}"

# 确保 EXE_PATH 存在
EXE_PATH=$(cd "$(dirname "${EXE_PATH}")" && pwd)/$(basename "${EXE_PATH}")
if [ ! -f "${EXE_PATH}" ]; then
    echo "Error: ${EXE_PATH} not found"
    exit 1
fi

# 生成 HTML 报告
xcrun llvm-cov show "${EXE_PATH}" -instr-profile="${PROFDATA}" -format=html > "${COVERAGE_HTML}"

# 生成覆盖率报告
xcrun llvm-cov report "${EXE_PATH}" -instr-profile="${PROFDATA}"

```

**方案二：**

```bash
#!/bin/bash

set -e  # 遇到错误立即退出

# 定义变量
SDK_PATH="/Applications/DevEco-Studio.app/Contents/sdk/HarmonyOS-NEXT-DB1"
CMAKE="${SDK_PATH}/openharmony/native/build-tools/cmake/bin/cmake"
SYSROOT="${SDK_PATH}/hms/native/sysroot"
TOOLCHAIN="${SDK_PATH}/hms/native/build/cmake/hmos.toolchain.cmake"
BUILD_DIR="build"
EXECUTABLE_FILE_NAME="entry_test"
LIB_DIR="lib"
REMOTE_DIR="/data/local/tmp"

# 更新依赖
ohpm install

# 创建并进入 build 目录
mkdir -p ${BUILD_DIR} && cd ${BUILD_DIR}

# 运行 cmake 命令
${CMAKE} -DBUILD_TESTS=ON -DBUILD_TESTS_COVERAGE=ON -DOHOS_STL=c++_shared \
         -DCMAKE_SYSROOT=${SYSROOT} -DOHOS_PLATFORM=OHOS -DOHOS_ARCH=arm64-v8a \
         -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN} ../..

# 构建项目
${CMAKE} --build .

# 推送动态链接库到设备上
find ${LIB_DIR} -type f -exec hdc file send {} ${REMOTE_DIR} \;

# 推送可执行测试文件到设备上
hdc shell rm -rf "${REMOTE_DIR}/${EXECUTABLE_FILE_NAME}"
hdc file send "${BUILD_DIR}/${EXECUTABLE_FILE_NAME}" ${REMOTE_DIR}
hdc shell chmod +x "${REMOTE_DIR}/${EXECUTABLE_FILE_NAME}"

# 运行测试
hdc shell "export LD_LIBRARY_PATH=${REMOTE_DIR}; ${REMOTE_DIR}/${EXECUTABLE_FILE_NAME}"

# 获取覆盖率数据
hdc file recv "${REMOTE_DIR}/default.profraw" .

# 生成覆盖率报告
xcrun llvm-profdata merge -sparse default.profraw -o default.profdata
pwd
xcrun llvm-cov show ${EXECUTABLE_FILE_NAME} -instr-profile=default.profdata -format=html > coverage.html
xcrun llvm-cov report ${EXECUTABLE_FILE_NAME} -instr-profile=default.profdata

# 清理临时文件
rm -f default.profraw default.profdata
```