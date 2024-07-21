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
