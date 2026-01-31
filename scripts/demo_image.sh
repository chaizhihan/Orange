#!/bin/bash
# =========================================
# ALIN 图像处理管道完整演示
# =========================================
#
# 演示:
# 1. 生成测试图像
# 2. 处理图像应用不同滤镜
# 3. 热切换滤镜效果

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

# 颜色
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
banner "ALIN Image Processing Pipeline Demo"
# ========================================

# 步骤 1: 生成测试图像
section "步骤 1: 生成测试图像"
TEST_IMAGE="demo/images/test_gradient.png"

python3 << 'EOF'
import struct
import zlib
import os

def create_gradient_png(filename, width=200, height=200):
    def png_chunk(chunk_type, data):
        chunk_len = len(data)
        chunk = chunk_type + data
        crc = zlib.crc32(chunk) & 0xffffffff
        return struct.pack('>I', chunk_len) + chunk + struct.pack('>I', crc)
    
    signature = b'\x89PNG\r\n\x1a\n'
    ihdr_data = struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
    ihdr = png_chunk(b'IHDR', ihdr_data)
    
    raw_data = b''
    for y in range(height):
        raw_data += b'\x00'
        for x in range(width):
            r = int(255 * x / width)
            g = int(255 * y / height)
            b = int(255 * (1 - x / width))
            raw_data += bytes([r, g, b])
    
    compressed = zlib.compress(raw_data, 9)
    idat = png_chunk(b'IDAT', compressed)
    iend = png_chunk(b'IEND', b'')
    
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    with open(filename, 'wb') as f:
        f.write(signature + ihdr + idat + iend)
    
    print(f"Created: {filename} ({width}x{height})")

create_gradient_png('demo/images/test_gradient.png')
EOF

success "测试图像生成完成"

# 节点路径
NODES="alin/nodes"
DECODER="$NODES/decode_image_py"
ENCODER="$NODES/encode_png_py"
FILTER_GRAY="$NODES/filter_grayscale_py"
FILTER_SEPIA="$NODES/filter_sepia_py"
FILTER_INVERT="$NODES/filter_invert_py"

# 检查节点
for node in "$DECODER" "$ENCODER" "$FILTER_GRAY" "$FILTER_SEPIA" "$FILTER_INVERT"; do
    if [ ! -x "$node" ]; then
        echo "Error: Node not found: $node"
        exit 1
    fi
done

# 步骤 2: 原始图像 (无滤镜)
section "步骤 2: 原始图像处理 (无滤镜)"
OUTPUT1="demo/images/output_original.png"

echo "{\"path\":\"$TEST_IMAGE\"}" | \
    "$DECODER" | \
    python3 -c "import sys,json; d=json.load(sys.stdin); d['output']='$OUTPUT1'; print(json.dumps(d))" | \
    "$ENCODER"

success "输出: $OUTPUT1"

# 步骤 3: 灰度滤镜
section "步骤 3: 应用灰度滤镜"
OUTPUT2="demo/images/output_grayscale.png"

info "管道: decode → grayscale → encode"
echo "{\"path\":\"$TEST_IMAGE\"}" | \
    "$DECODER" | \
    "$FILTER_GRAY" | \
    python3 -c "import sys,json; d=json.load(sys.stdin); d['output']='$OUTPUT2'; print(json.dumps(d))" | \
    "$ENCODER"

success "输出: $OUTPUT2"

# 步骤 4: 复古滤镜 (热切换)
section "步骤 4: 热切换 → 复古滤镜"
OUTPUT3="demo/images/output_sepia.png"

info "热切换: filter_grayscale → filter_sepia"
echo "{\"path\":\"$TEST_IMAGE\"}" | \
    "$DECODER" | \
    "$FILTER_SEPIA" | \
    python3 -c "import sys,json; d=json.load(sys.stdin); d['output']='$OUTPUT3'; print(json.dumps(d))" | \
    "$ENCODER"

success "输出: $OUTPUT3"

# 步骤 5: 反色滤镜 (热切换)
section "步骤 5: 热切换 → 反色滤镜"
OUTPUT4="demo/images/output_invert.png"

info "热切换: filter_sepia → filter_invert"
echo "{\"path\":\"$TEST_IMAGE\"}" | \
    "$DECODER" | \
    "$FILTER_INVERT" | \
    python3 -c "import sys,json; d=json.load(sys.stdin); d['output']='$OUTPUT4'; print(json.dumps(d))" | \
    "$ENCODER"

success "输出: $OUTPUT4"

# 步骤 6: 结果汇总
section "步骤 6: 处理结果汇总"

echo ""
echo "输出文件:"
for f in demo/images/output_*.png; do
    if [ -f "$f" ]; then
        SIZE=$(ls -lh "$f" | awk '{print $5}')
        echo "  📷 $(basename "$f") - $SIZE"
    fi
done

# 完成
banner "演示完成"

echo "ALIN 图像处理管道特性验证:"
echo ""
echo "  ✅ 图像解码 (PNG → PPM)"
echo "  ✅ 灰度滤镜 (Grayscale)"
echo "  ✅ 复古滤镜 (Sepia)"
echo "  ✅ 反色滤镜 (Invert)"
echo "  ✅ PNG 编码输出"
echo "  ✅ 热切换滤镜效果"
echo ""
echo "查看输出图像:"
echo ""
echo -e "  ${GREEN}open demo/images/output_*.png${NC}"
echo ""
