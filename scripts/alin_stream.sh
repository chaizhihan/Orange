#!/bin/bash
# =========================================
# ALIN 流处理驱动 (Stream Processing Driver)
# =========================================
#
# 功能:
# 1. 持续从 stdin 或文件读取日志
# 2. 按行处理，流经当前拓扑
# 3. 支持批处理和实时模式
# 4. 状态检查点
#
# 使用方式:
#   cat logs.jsonl | ./scripts/alin_stream.sh
#   ./scripts/alin_stream.sh demo/sample_logs/app.log
#   tail -f /var/log/app.log | ./scripts/alin_stream.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ACTIVE_DIR="$PROJECT_DIR/alin/active"
STATE_DIR="$PROJECT_DIR/alin/state"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[STREAM]${NC} $1" >&2; }
log_success() { echo -e "${GREEN}[STREAM]${NC} $1" >&2; }
log_error() { echo -e "${RED}[STREAM]${NC} $1" >&2; }

# 配置
BATCH_SIZE=${ALIN_BATCH_SIZE:-1}
VERBOSE=${ALIN_VERBOSE:-0}

# 确保状态目录存在
mkdir -p "$STATE_DIR"

# 检查 active 目录
if [ ! -d "$ACTIVE_DIR" ]; then
    log_error "Active directory not found: $ACTIVE_DIR"
    exit 1
fi

# 扫描拓扑
get_pipeline() {
    local NODES=()
    local LINKS=$(ls -1 "$ACTIVE_DIR" 2>/dev/null | sort)
    
    if [ -z "$LINKS" ]; then
        log_error "No active links found"
        exit 1
    fi
    
    for link in $LINKS; do
        local link_path="$ACTIVE_DIR/$link"
        if [ -L "$link_path" ]; then
            local target=$(readlink "$link_path")
            if [ -x "$target" ]; then
                NODES+=("$target")
            fi
        fi
    done
    
    echo "${NODES[@]}"
}

# 显示拓扑
show_topology() {
    log_info "=== Stream Processing Topology ==="
    for link in $(ls -1 "$ACTIVE_DIR" 2>/dev/null | sort); do
        local link_path="$ACTIVE_DIR/$link"
        if [ -L "$link_path" ]; then
            local target=$(readlink "$link_path")
            local inode=$(ls -i "$target" 2>/dev/null | awk '{print $1}')
            log_info "  $link → $(basename "$target") [Inode: $inode]"
        fi
    done
    log_info "=================================="
}

# 处理单行
process_line() {
    local line="$1"
    local result="$line"
    
    # 获取管道节点
    local nodes=($(get_pipeline))
    
    # 设置环境变量
    export ALIN_STATE_FILE="$STATE_DIR/agg_count.state"
    
    # 流经每个节点
    for node in "${nodes[@]}"; do
        if [ -n "$result" ]; then
            result=$(echo "$result" | "$node" 2>/dev/null || echo "")
        fi
    done
    
    echo "$result"
}

# 批量处理
process_batch() {
    local count=0
    local start_time=$(date +%s)
    
    while IFS= read -r line || [ -n "$line" ]; do
        if [ -z "$line" ]; then
            continue
        fi
        
        result=$(process_line "$line")
        
        ((count++))
        
        if [ $((count % 100)) -eq 0 ]; then
            local elapsed=$(($(date +%s) - start_time))
            elapsed=$((elapsed == 0 ? 1 : elapsed))
            local rate=$((count / elapsed))
            log_info "Processed: $count events (${rate}/sec)"
        fi
    done
    
    local total_time=$(($(date +%s) - start_time))
    total_time=$((total_time == 0 ? 1 : total_time))
    log_success "=== Stream Complete ==="
    log_success "Total events: $count"
    log_success "Duration: ${total_time}s"
    log_success "Avg rate: $((count / total_time)) events/sec"
}

# 主入口
main() {
    show_topology
    
    if [ -n "$1" ] && [ -f "$1" ]; then
        # 从文件读取
        log_info "Reading from file: $1"
        cat "$1" | process_batch
    else
        # 从 stdin 读取
        log_info "Reading from stdin..."
        process_batch
    fi
}

main "$@"
