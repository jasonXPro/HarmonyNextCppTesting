# the minimum version of CMake.
cmake_minimum_required(VERSION 3.4.1)
project(HMGoogleTestDemo)

set(NATIVERENDER_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(DEFINED PACKAGE_FIND_FILE)
    include(${PACKAGE_FIND_FILE})
endif()

include_directories(${NATIVERENDER_ROOT_PATH}
                    ${NATIVERENDER_ROOT_PATH}/include)
file(GLOB SOURCE_CORE napi_init.cpp my_math.h)


# GoogleTest Start
# 添加一个选项来控制是否编译测试 修改 build-profile.json5 中 buildOption->externalNativeOptions->"arguments": "-DBUILD_TESTS=ON" 即可开启
if(DEFINED BUILD_TESTS AND CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "BUILD_TESTS is ${BUILD_TESTS}")
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
    if(DEFINED BUILD_TESTS_COVERAGE)
        # 创建一个测试可执行文件
        add_executable(entry_test ${SOURCE_CORE} ${TESTS_CODE})
        # 链接 Google Test 库
        target_link_libraries(entry_test libace_napi.z.so libhilog_ndk.z.so gtest_main gmock_main)
        #配置断言失效
        target_compile_definitions(entry_test PRIVATE NDEBUG)
        #单测覆盖率配置
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate=/data/local/tmp/default.profraw -fcoverage-mapping -Wno-deprecated-declarations")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")
    else()
        add_library(entry SHARED ${SOURCE_CORE} ${TESTS_CODE})
        target_link_libraries(entry PUBLIC libace_napi.z.so libhilog_ndk.z.so gtest_main gmock_main)
        #配置断言失效
        target_compile_definitions(entry PRIVATE NDEBUG)
        #单测覆盖率配置
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate=/data/storage/el2/base/haps/entry/cache/default.profraw -fcoverage-mapping -Wno-deprecated-declarations")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate")

    endif()
# GoogleTest End
else()
    add_library(entry SHARED napi_init.cpp)
    target_link_libraries(entry PUBLIC libace_napi.z.so)
endif()