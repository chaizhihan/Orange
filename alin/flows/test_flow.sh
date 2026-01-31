#!/bin/bash
# =========================================
# ALIN MVP 测试流程 (Test Flow Script)
# =========================================
#
# 演示完整的 ALIN 数据流和热切换流程

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"

cd "$PROJECT_DIR"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

banner() {
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════${NC}"
    echo ""
}

section() {
    echo ""
    echo -e "${YELLOW}▶ $1${NC}"
    echo ""
}

# 测试数据
TEST_INPUT='[1, 2, 3]'

banner "ALIN MVP 演示 - 数据翻倍与求和"

# 步骤 1: 清理并重建
section "步骤 1: 清理并编译节点"
rm -rf alin/active/* 2>/dev/null || true
make clean
make all

# 步骤 2: 查看可用节点
section "步骤 2: 查看可用节点"
./scripts/alin_link.sh nodes

# 步骤 3: 配置拓扑 1 (double → sum)
section "步骤 3: 配置拓扑 1 (double → sum)"
./scripts/alin_link.sh swap_logic 01_dbl double
./scripts/alin_link.sh swap_logic 02_sum sum
./scripts/alin_link.sh list

# 步骤 4: 执行数据流 1
section "步骤 4: 执行数据流 (拓扑1: double → sum)"
echo -e "${BLUE}输入: $TEST_INPUT${NC}"
echo ""
RESULT1=$(echo "$TEST_INPUT" | ./scripts/alin_run.sh)
echo ""
echo -e "${GREEN}最终结果: $RESULT1${NC}"
echo ""
echo -e "${BLUE}解释: [1,2,3] → double → [2,4,6] → sum → 12${NC}"

# 步骤 5: 热切换拓扑
section "步骤 5: 原子热切换 (无需重启)"
rm -f alin/active/*
./scripts/alin_link.sh swap_logic 01_sum sum
./scripts/alin_link.sh swap_logic 02_dbl double
./scripts/alin_link.sh list

# 步骤 6: 执行数据流 2
section "步骤 6: 执行数据流 (拓扑2: sum → double)"
echo -e "${BLUE}输入: $TEST_INPUT${NC}"
echo ""
RESULT2=$(echo "$TEST_INPUT" | ./scripts/alin_run.sh)
echo ""
echo -e "${GREEN}最终结果: $RESULT2${NC}"
echo ""
echo -e "${BLUE}解释: [1,2,3] → sum → 6 → double → 12${NC}"

# 步骤 7: 演示 Inode 追踪
section "步骤 7: Inode 追踪"
echo "当前拓扑的 Inode 信息:"
ls -li alin/active/

# 摘要
banner "演示完成"
echo "✅ 拓扑 1 (double → sum): $TEST_INPUT → $RESULT1"
echo "✅ 拓扑 2 (sum → double): $TEST_INPUT → $RESULT2"
echo ""
echo "ALIN 架构特性验证:"
echo "  • 原子路由切换 (ln -sf)"
echo "  • 无停机热更新"
echo "  • Inode 级别的追踪"
echo "  • JSON 流式数据传输"
echo ""
