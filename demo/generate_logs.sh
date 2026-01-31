#!/bin/bash
# =========================================
# ALIN 日志生成器 (Log Generator)
# =========================================
#
# 功能: 生成模拟的 JSON 日志用于测试流处理管道
#
# 使用方式:
#   ./demo/generate_logs.sh 100           # 生成 100 条
#   ./demo/generate_logs.sh 1000 > logs.jsonl

set -e

COUNT=${1:-100}
SEED=${2:-$$}

# 日志级别和权重
LEVELS=("DEBUG" "DEBUG" "INFO" "INFO" "INFO" "INFO" "WARN" "WARN" "ERROR" "FATAL")
SERVICES=("api" "auth" "database" "cache" "queue" "worker" "gateway" "scheduler")
MESSAGES=(
    "Request processed successfully"
    "User login attempt"
    "Database query completed"
    "Cache miss, fetching from source"
    "Connection timeout, retrying"
    "Invalid request parameters"
    "Authentication failed"
    "Rate limit exceeded"
    "Memory usage high"
    "Disk space low"
    "Service health check passed"
    "Background job started"
    "Task completed"
    "Configuration reloaded"
    "Connection pool exhausted"
    "Unhandled exception occurred"
    "Request validation failed"
    "Session expired"
    "API rate limit warning"
    "Critical system error detected"
)

ERROR_MESSAGES=(
    "Connection refused to database"
    "Out of memory exception"
    "Stack overflow in worker thread"
    "Null pointer exception"
    "Timeout waiting for response"
    "Disk write failed"
    "Network unreachable"
    "SSL certificate expired"
    "Authentication token invalid"
    "Critical assertion failed"
)

# 随机函数 (使用 $RANDOM)
random_element() {
    local arr=("$@")
    echo "${arr[$((RANDOM % ${#arr[@]}))]}"
}

random_int() {
    local min=$1
    local max=$2
    echo $((RANDOM % (max - min + 1) + min))
}

# 生成日志
for i in $(seq 1 $COUNT); do
    level=$(random_element "${LEVELS[@]}")
    service=$(random_element "${SERVICES[@]}")
    
    if [ "$level" == "ERROR" ] || [ "$level" == "FATAL" ]; then
        message=$(random_element "${ERROR_MESSAGES[@]}")
    else
        message=$(random_element "${MESSAGES[@]}")
    fi
    
    # 生成时间戳 (当前时间附近)
    offset=$(random_int -3600 0)
    timestamp=$(($(date +%s) + offset))
    
    # 生成请求 ID
    request_id=$(printf "%08x" $((RANDOM * RANDOM)))
    
    # 生成延迟 (毫秒)
    latency=$(random_int 1 5000)
    
    # 输出 JSON
    echo "{\"level\":\"$level\",\"service\":\"$service\",\"msg\":\"$message\",\"timestamp\":$timestamp,\"request_id\":\"$request_id\",\"latency_ms\":$latency}"
    
    # 可选延迟 (模拟实时日志)
    if [ -n "$ALIN_DELAY" ]; then
        sleep "$ALIN_DELAY"
    fi
done
