#!/bin/bash

LOGFILE="build_and_run.log"

echo "Starting build and run..." > $LOGFILE

# 进入 build 目录
cd build
echo "Entering build directory..." >> $LOGFILE

# 运行 cmake 和 make
cmake .. && make -j$(nproc) >> $LOGFILE 2>&1

echo "Finished build and run" >> $LOGFILE

# 执行完后退出
exit 0