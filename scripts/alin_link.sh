#!/bin/bash
# =========================================
# ALIN 热更新脚本 (Atomic Linkage Controller)
# =========================================
#
# 功能:
# - swap_logic: 原子性地修改路由指向
# - health_check: 验证节点可用性
# - rollback: 回滚到上一个稳定版本
# - list: 列出当前拓扑
#
# 使用方式:
#   ./alin_link.sh swap_logic 01_dbl double
#   ./alin_link.sh health_check 01_dbl
#   ./alin_link.sh rollback 01_dbl
#   ./alin_link.sh list

set -e

# 配置
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ACTIVE_DIR="$PROJECT_DIR/alin/active"
NODES_DIR="$PROJECT_DIR/alin/nodes"
BACKUP_DIR="$PROJECT_DIR/alin/.backup"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[OK]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }

# 确保目录存在
mkdir -p "$ACTIVE_DIR" "$BACKUP_DIR"

# 查找节点 (支持名称前缀匹配)
find_node() {
    local name="$1"
    # 精确匹配或前缀匹配
    local found=$(ls -1 "$NODES_DIR" 2>/dev/null | grep -E "^${name}(_|$)" | head -1)
    
    if [ -n "$found" ]; then
        echo "$NODES_DIR/$found"
    fi
}

# 获取当前链接的目标
get_current_target() {
    local alias="$1"
    local link_path="$ACTIVE_DIR/$alias"
    
    if [ -L "$link_path" ]; then
        readlink "$link_path"
    fi
}

# swap_logic: 原子切换逻辑指向
cmd_swap_logic() {
    local alias="$1"
    local target="$2"
    
    if [ -z "$alias" ] || [ -z "$target" ]; then
        log_error "Usage: swap_logic <alias> <target_node>"
    fi
    
    # 查找目标节点
    local node_path=$(find_node "$target")
    if [ -z "$node_path" ] || [ ! -x "$node_path" ]; then
        log_error "Node not found or not executable: $target"
    fi
    
    local link_path="$ACTIVE_DIR/$alias"
    
    # 备份当前链接
    if [ -L "$link_path" ]; then
        local current=$(readlink "$link_path")
        echo "$current" > "$BACKUP_DIR/${alias}.prev"
        log_info "Backed up previous link: $(basename "$current")"
    fi
    
    # 原子切换 (ln -sf 是原子操作)
    log_info "Switching: $alias -> $(basename "$node_path")"
    ln -sf "$node_path" "$link_path"
    
    # 获取新的 Inode
    local inode=$(ls -i "$node_path" | awk '{print $1}')
    log_success "Atomic swap complete! [Inode: $inode]"
}

# health_check: 验证节点可用性
cmd_health_check() {
    local alias="$1"
    
    if [ -z "$alias" ]; then
        log_error "Usage: health_check <alias>"
    fi
    
    local link_path="$ACTIVE_DIR/$alias"
    
    if [ ! -L "$link_path" ]; then
        log_error "Link not found: $alias"
    fi
    
    local target=$(readlink "$link_path")
    
    if [ ! -x "$target" ]; then
        log_error "Target not executable: $target"
    fi
    
    # 测试节点
    log_info "Testing node: $(basename "$target")"
    local test_result=$(echo '[1]' | "$target" 2>/dev/null)
    
    if [ -n "$test_result" ]; then
        log_success "Health check passed! Response: $test_result"
        return 0
    else
        log_error "Health check failed!"
    fi
}

# rollback: 回滚到上一个版本
cmd_rollback() {
    local alias="$1"
    
    if [ -z "$alias" ]; then
        log_error "Usage: rollback <alias>"
    fi
    
    local backup_file="$BACKUP_DIR/${alias}.prev"
    
    if [ ! -f "$backup_file" ]; then
        log_error "No backup found for: $alias"
    fi
    
    local prev_target=$(cat "$backup_file")
    
    if [ ! -x "$prev_target" ]; then
        log_error "Previous target not executable: $prev_target"
    fi
    
    log_info "Rolling back: $alias -> $(basename "$prev_target")"
    ln -sf "$prev_target" "$ACTIVE_DIR/$alias"
    
    log_success "Rollback complete!"
}

# list: 列出当前拓扑
cmd_list() {
    echo ""
    echo "=== ALIN Active Topology ==="
    echo ""
    
    if [ -z "$(ls -A "$ACTIVE_DIR" 2>/dev/null)" ]; then
        log_warn "No active links found"
        return
    fi
    
    printf "%-20s %-30s %s\n" "ALIAS" "TARGET" "INODE"
    printf "%-20s %-30s %s\n" "-----" "------" "-----"
    
    for link in $(ls -1 "$ACTIVE_DIR" | sort); do
        local link_path="$ACTIVE_DIR/$link"
        if [ -L "$link_path" ]; then
            local target=$(readlink "$link_path")
            local target_name=$(basename "$target")
            local inode=$(ls -i "$target" 2>/dev/null | awk '{print $1}')
            printf "%-20s %-30s %s\n" "$link" "$target_name" "$inode"
        fi
    done
    
    echo ""
}

# 列出可用节点
cmd_nodes() {
    echo ""
    echo "=== Available Nodes ==="
    echo ""
    
    if [ -z "$(ls -A "$NODES_DIR" 2>/dev/null)" ]; then
        log_warn "No nodes found. Run 'make all' to compile nodes."
        return
    fi
    
    printf "%-30s %s\n" "NODE" "INODE"
    printf "%-30s %s\n" "----" "-----"
    
    for node in $(ls -1 "$NODES_DIR" | sort); do
        local node_path="$NODES_DIR/$node"
        local inode=$(ls -i "$node_path" 2>/dev/null | awk '{print $1}')
        printf "%-30s %s\n" "$node" "$inode"
    done
    
    echo ""
}

# 帮助信息
cmd_help() {
    echo ""
    echo "ALIN Link Controller - 原子链接管理器"
    echo ""
    echo "Usage: $0 <command> [arguments]"
    echo ""
    echo "Commands:"
    echo "  swap_logic <alias> <target>  原子切换链接指向"
    echo "  health_check <alias>         验证节点可用性"
    echo "  rollback <alias>             回滚到上一个版本"
    echo "  list                         列出当前拓扑"
    echo "  nodes                        列出可用节点"
    echo "  help                         显示帮助信息"
    echo ""
    echo "Examples:"
    echo "  $0 swap_logic 01_dbl double"
    echo "  $0 swap_logic 02_sum sum"
    echo "  $0 health_check 01_dbl"
    echo "  $0 list"
    echo ""
}

# 主入口
case "$1" in
    swap_logic)
        cmd_swap_logic "$2" "$3"
        ;;
    health_check)
        cmd_health_check "$2"
        ;;
    rollback)
        cmd_rollback "$2"
        ;;
    list)
        cmd_list
        ;;
    nodes)
        cmd_nodes
        ;;
    help|--help|-h)
        cmd_help
        ;;
    *)
        cmd_help
        exit 1
        ;;
esac
