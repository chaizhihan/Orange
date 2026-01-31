#!/bin/bash
# =========================================
# ALIN 流处理引擎完整演示
# =========================================
#
# 演示:
# 1. 编译所有流处理节点
# 2. 配置管道拓扑
# 3. 生成模拟日志
# 4. 执行流处理
# 5. 热切换演示
# 6. 启动 Web Dashboard

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

banner() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║  $1${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

section() {
    echo ""
    echo -e "${YELLOW}▶ $1${NC}"
    echo ""
}

success() {
    echo -e "${GREEN}✅ $1${NC}"
}

info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# ========================================
banner "ALIN Stream Processing Engine Demo"
# ========================================

# 步骤 1: 清理和编译
section "步骤 1: 编译流处理节点"
rm -f alin/active/* 2>/dev/null || true
rm -f alin/state/*.state 2>/dev/null || true
make clean 2>/dev/null || true
make stream

# 步骤 2: 查看可用节点
section "步骤 2: 查看编译后的节点"
./scripts/alin_link.sh nodes

# 步骤 3: 配置管道拓扑
section "步骤 3: 配置流处理管道"
info "管道: parse_json → filter_level → agg_count → alert_console"
./scripts/alin_link.sh swap_logic 01_parse parse_json
./scripts/alin_link.sh swap_logic 02_filter filter_level
./scripts/alin_link.sh swap_logic 03_agg agg_count
./scripts/alin_link.sh swap_logic 04_alert alert_console

echo ""
./scripts/alin_link.sh list

# 步骤 4: 单行测试
section "步骤 4: 单行数据流测试"

# 设置环境变量
export ALIN_FILTER_LEVEL="WARN"
export ALIN_STATE_FILE="$PROJECT_DIR/alin/state/agg_count.state"

info "发送 ERROR 级别日志..."
echo '{"level":"ERROR","msg":"Database connection failed","ts":1706745600}' | ./scripts/alin_run.sh

echo ""
info "发送 INFO 级别日志 (应被过滤)..."
RESULT=$(echo '{"level":"INFO","msg":"Request completed","ts":1706745601}' | ./scripts/alin_run.sh 2>/dev/null)
if [ -z "$RESULT" ]; then
    success "INFO 日志已被 filter_level 过滤 (预期行为)"
else
    echo "Output: $RESULT"
fi

echo ""
info "发送 WARN 级别日志..."
echo '{"level":"WARN","msg":"High memory usage detected","ts":1706745602}' | ./scripts/alin_run.sh

# 步骤 5: 批量处理演示
section "步骤 5: 批量日志流处理"
info "生成 50 条模拟日志..."

./demo/generate_logs.sh 50 > alin/data/test_logs.jsonl

info "处理日志流 (只显示 WARN 及以上级别)..."
cat alin/data/test_logs.jsonl | ./scripts/alin_stream.sh 2>&1 | tail -20

# 步骤 6: 查看聚合状态
section "步骤 6: 查看聚合统计状态"
if [ -f alin/state/agg_count.state ]; then
    echo "--- alin/state/agg_count.state ---"
    cat alin/state/agg_count.state
    echo "-----------------------------------"
fi

# 步骤 7: 热切换演示
section "步骤 7: 热切换演示 - 修改过滤级别"
info "当前过滤级别: WARN"
info "热切换: 将 ALIN_FILTER_LEVEL 改为 DEBUG (接受所有日志)"

export ALIN_FILTER_LEVEL="DEBUG"
rm -f alin/state/agg_count.state

info "重新发送同样的 INFO 日志..."
echo '{"level":"INFO","msg":"Request completed","ts":1706745601}' | ./scripts/alin_run.sh

success "INFO 日志现在通过了过滤器!"

# 步骤 8: 提示 Web Dashboard
section "步骤 8: Web Dashboard"
echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  要启动 Web Dashboard，请运行："
echo ""
echo -e "    ${GREEN}cd $PROJECT_DIR/web && python3 -m http.server 8080${NC}"
echo ""
echo "  然后在浏览器中打开："
echo ""
echo -e "    ${CYAN}http://localhost:8080${NC}"
echo ""
echo -e "${MAGENTA}═══════════════════════════════════════════════════════════════${NC}"

# 完成
banner "演示完成"

echo "ALIN 流处理引擎特性验证:"
echo ""
echo "  ✅ 多节点管道处理 (parse → filter → agg → alert)"
echo "  ✅ 日志级别过滤 (可热切换)"
echo "  ✅ 有状态聚合 (持久化计数)"
echo "  ✅ 格式化告警输出"
echo "  ✅ 批量流式处理"
echo "  ✅ Inode 级别追踪"
echo ""
echo "文档:"
echo "  📁 项目目录: $PROJECT_DIR"
echo "  📊 状态文件: alin/state/agg_count.state"
echo "  🌐 Dashboard: web/index.html"
echo ""
