#!/bin/bash
# =========================================
# ALIN 调度引擎 (Shell Orchestrator)
# =========================================
#
# 功能:
# 1. 拓扑扫描: 读取 /alin/active 下的 Inode 列表
# 2. 动态管道组装: 按字母顺序构建管道链
# 3. 状态透明化: 输出每个路径指向的 Inode 编号
#
# 使用方式:
#   echo '[1,2,3]' | ./scripts/alin_run.sh
#   cat data.json | ./scripts/alin_run.sh
#   ./scripts/alin_run.sh < input.json

set -e

# 配置
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ACTIVE_DIR="$PROJECT_DIR/alin/active"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1" >&2
}

log_success() {
    echo -e "${GREEN}[OK]${NC} $1" >&2
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

log_topology() {
    echo -e "${YELLOW}[TOPOLOGY]${NC} $1" >&2
}

# 检查 active 目录是否存在
if [ ! -d "$ACTIVE_DIR" ]; then
    log_error "Active directory not found: $ACTIVE_DIR"
    exit 1
fi

# 扫描拓扑
log_info "Scanning topology at: $ACTIVE_DIR"

# 获取所有链接,按名称排序
LINKS=$(ls -1 "$ACTIVE_DIR" 2>/dev/null | sort)

if [ -z "$LINKS" ]; then
    log_error "No active links found in $ACTIVE_DIR"
    exit 1
fi

# 收集节点路径和显示拓扑信息
NODES=()
log_topology "=== Current Pipeline Topology ==="

for link in $LINKS; do
    link_path="$ACTIVE_DIR/$link"
    
    if [ -L "$link_path" ]; then
        # 获取实际目标
        target=$(readlink "$link_path")
        # 获取 Inode 号
        inode=$(ls -i "$link_path" 2>/dev/null | awk '{print $1}')
        target_inode=$(ls -i "$target" 2>/dev/null | awk '{print $1}')
        
        log_topology "  $link -> $(basename "$target") [Inode: $target_inode]"
        
        # 检查目标是否可执行
        if [ -x "$target" ]; then
            NODES+=("$target")
        else
            log_error "Target not executable: $target"
            exit 1
        fi
    else
        log_error "Not a symbolic link: $link_path"
        exit 1
    fi
done

log_topology "================================="
log_info "Pipeline: ${#NODES[@]} nodes"

# 构建管道命令
if [ ${#NODES[@]} -eq 0 ]; then
    log_error "No valid nodes found"
    exit 1
fi

# 读取输入
INPUT=$(cat)

if [ -z "$INPUT" ]; then
    log_error "No input data received"
    exit 1
fi

log_info "Input: $INPUT"

# 执行管道
RESULT="$INPUT"
for i in "${!NODES[@]}"; do
    node="${NODES[$i]}"
    node_name=$(basename "$node")
    
    log_info "Processing through: $node_name"
    RESULT=$(echo "$RESULT" | "$node")
    
    if [ $? -ne 0 ]; then
        log_error "Node failed: $node_name"
        exit 1
    fi
    
    log_success "Output from $node_name: $RESULT"
done

# 输出最终结果
log_success "=== Final Result ==="
echo "$RESULT"
