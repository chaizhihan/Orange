#!/bin/bash
# alin_loader.sh - The "JTAG" for ALin Architecture
# Usage: ./alin_loader.sh circuit_config.json

BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NODES_DIR="$BASE_DIR/alin/nodes"
ACTIVE_DIR="$BASE_DIR/alin/active"

# 检查依赖
if ! command -v jq &> /dev/null; then
    echo "[Error] jq is required to parse bitstreams. Please install it (e.g., brew install jq)."
    exit 1
fi

BITSTREAM=$1

if [ ! -f "$BITSTREAM" ]; then
    echo "[Error] Bitstream file not found: $BITSTREAM"
    exit 1
fi

echo "=========================================="
echo "⚡ ALin FPGA-Style Loader v1.0"
echo "📂 Loading Bitstream: $BITSTREAM"
echo "=========================================="

# 1. 解析 Bitstream 元数据
project_name=$(jq -r '.project' "$BITSTREAM")
version=$(jq -r '.version' "$BITSTREAM")

echo ">> Circuit Name: $project_name (Rev: $version)"
echo ">> Resetting Active Logic Gates..."

# 2. 模拟 FPGA 的全片擦除 (可选，或仅覆盖)
# mkdir -p $ACTIVE_DIR
# rm -f $ACTIVE_DIR/* 

# 3. 开始逐个插槽“烧录” (Routing)
jq -r '.topology | to_entries[] | "\(.key) \(.value)"' "$BITSTREAM" | while read -r slot logic_id; do
    
    # 在 Nodes 仓库中寻找对应的二进制 (支持模糊匹配 name_hash)
    # 逻辑：找到最新的那个版本
    target_inode=$(ls -t $NODES_DIR/${logic_id}* 2>/dev/null | head -n 1)

    if [ -z "$target_inode" ]; then
        echo "❌ [Error] Logic Cell not found for ID: $logic_id"
        continue
    fi

    target_filename=$(basename "$target_inode")
    
    # 原子链接切换 (The Atomic Flash)
    mkdir -p "$ACTIVE_DIR"
    ln -sf "$target_inode" "$ACTIVE_DIR/$slot"
    
    # 模拟硬件烧录的微小延迟 (视觉效果)
    sleep 0.1
    echo "🟢 [Flashed] Slot [$slot] <== $target_filename"
done

echo "=========================================="
echo "✅ Bitstream Loaded. Circuit is LIVE."
echo "=========================================="
