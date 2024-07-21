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
