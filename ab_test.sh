#!/bin/bash

# 定义结果文件
RESULT_FILE="ab_test_result_$(date +%Y%m%d%H%M%S).log"

# 执行 ab 测试并记录结果到文件
echo "Starting Apache Benchmark test..." > $RESULT_FILE
sudo ab -n 1000000 -c 1000 -k http://127.0.0.1:8080/ >> $RESULT_FILE 2>&1

echo "Test completed. Results are saved in $RESULT_FILE"
