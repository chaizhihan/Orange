#!/bin/bash
# =========================================
# ALIN 图像处理驱动 (Image Processing Driver)
# =========================================
#
# 功能:
# 1. 解码输入图像
# 2. 应用当前拓扑中的所有滤镜
# 3. 编码输出图像
#
# 使用方式:
#   ./scripts/alin_image.sh input.jpg output.png
#   ./scripts/alin_image.sh input.jpg  # 输出到 /tmp

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ACTIVE_DIR="$PROJECT_DIR/alin/active"
NODES_DIR="$PROJECT_DIR/alin/nodes"

# 颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[IMAGE]${NC} $1" >&2; }
log_success() { echo -e "${GREEN}[IMAGE]${NC} $1" >&2; }
log_error() { echo -e "${RED}[IMAGE]${NC} $1" >&2; }

# 参数
INPUT_FILE="$1"
OUTPUT_FILE="${2:-/tmp/alin_output_$$.png}"

if [ -z "$INPUT_FILE" ]; then
    echo "Usage: $0 <input_image> [output_image]"
    echo ""
    echo "Example:"
    echo "  $0 demo/images/sample.jpg output.png"
    exit 1
fi

if [ ! -f "$INPUT_FILE" ]; then
    log_error "Input file not found: $INPUT_FILE"
    exit 1
fi

# 获取绝对路径
INPUT_FILE="$(cd "$(dirname "$INPUT_FILE")" && pwd)/$(basename "$INPUT_FILE")"
if [[ "$OUTPUT_FILE" != /* ]]; then
    OUTPUT_FILE="$(pwd)/$OUTPUT_FILE"
fi

log_info "Input: $INPUT_FILE"
log_info "Output: $OUTPUT_FILE"

# 查找节点 (支持 C 编译版和 Python 版)
find_node() {
    local name="$1"
    # 先找 C 编译版
    local c_node=$(ls -1 "$NODES_DIR" 2>/dev/null | grep -E "^${name}_[a-f0-9]+$" | head -1)
    if [ -n "$c_node" ]; then
        echo "$c_node"
        return
    fi
    # 再找 Python 版
    local py_node=$(ls -1 "$NODES_DIR" 2>/dev/null | grep -E "^${name}_py$" | head -1)
    if [ -n "$py_node" ]; then
        echo "$py_node"
        return
    fi
}

# 获取管道节点
get_filter_nodes() {
    local nodes=()
    if [ -d "$ACTIVE_DIR" ]; then
        for link in $(ls -1 "$ACTIVE_DIR" 2>/dev/null | sort); do
            local link_path="$ACTIVE_DIR/$link"
            if [ -L "$link_path" ]; then
                local target=$(readlink "$link_path")
                if [ -x "$target" ]; then
                    nodes+=("$target")
                fi
            fi
        done
    fi
    echo "${nodes[@]}"
}

# 显示拓扑
log_info "=== Image Processing Topology ==="

DECODER=$(find_node "decode_image")
ENCODER=$(find_node "encode_png")

if [ -z "$DECODER" ]; then
    log_error "Decoder not found. Run: make decode_image"
    exit 1
fi

if [ -z "$ENCODER" ]; then
    log_error "Encoder not found. Run: make encode_png"
    exit 1
fi

log_info "Decoder: $DECODER"
log_info "Encoder: $ENCODER"

# 获取过滤器节点
FILTER_NODES=($(get_filter_nodes))
if [ ${#FILTER_NODES[@]} -gt 0 ]; then
    log_info "Filters:"
    for node in "${FILTER_NODES[@]}"; do
        log_info "  - $(basename "$node")"
    done
else
    log_info "Filters: (none - using passthrough)"
fi
log_info "================================="

# 处理流程
log_info "Processing..."

# Step 1: 解码
DECODED=$(echo "{\"path\":\"$INPUT_FILE\"}" | "$NODES_DIR/$DECODER")

if [ -z "$DECODED" ] || [[ "$DECODED" != *"ppm"* ]]; then
    log_error "Decode failed"
    exit 1
fi

log_success "Decoded: $(echo "$DECODED" | grep -o '"width":[0-9]*' | head -1), $(echo "$DECODED" | grep -o '"height":[0-9]*' | head -1)"

# Step 2: 应用滤镜
CURRENT="$DECODED"
for node in "${FILTER_NODES[@]}"; do
    node_name=$(basename "$node")
    log_info "Applying: $node_name"
    CURRENT=$(echo "$CURRENT" | "$node")
    
    if [ -z "$CURRENT" ]; then
        log_error "Filter failed: $node_name"
        exit 1
    fi
    
    # 提取滤镜名
    filter_name=$(echo "$CURRENT" | grep -o '"filter":"[^"]*"' | cut -d'"' -f4)
    if [ -n "$filter_name" ]; then
        log_success "Applied: $filter_name"
    fi
done

# Step 3: 编码
log_info "Encoding to: $OUTPUT_FILE"
RESULT=$(echo "{\"output\":\"$OUTPUT_FILE\",$(echo "$CURRENT" | sed 's/^{//')}" | "$NODES_DIR/$ENCODER")

if [[ "$RESULT" == *'"success":true'* ]]; then
    log_success "=== Processing Complete ==="
    log_success "Output saved to: $OUTPUT_FILE"
    
    # 显示文件信息
    if [ -f "$OUTPUT_FILE" ]; then
        SIZE=$(ls -lh "$OUTPUT_FILE" | awk '{print $5}')
        log_success "File size: $SIZE"
    fi
else
    log_error "Encoding failed"
    echo "$RESULT"
    exit 1
fi
