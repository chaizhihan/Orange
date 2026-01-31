# ALIN Makefile - 原子节点编译系统 (Stream Processing Edition)
# 
# 功能:
# - 支持多目录源码结构 (parsers, filters, aggregators, alerters)
# - 自动计算源码 MD5 hash (仅取前8位)
# - 输出格式: [name]_[hash]
# - 自动移动到 alin/nodes/
# - 自动生成 .meta 元数据文件
#
# 使用方式:
#   make parse_json   # 编译单个节点
#   make all          # 编译所有节点
#   make stream       # 编译所有流处理节点
#   make clean        # 清理编译产物

CC = clang
CFLAGS = -Wall -O2
NODES_DIR = alin/nodes
META_DIR = alin/meta

# 源码目录
SRC_DIRS = alin/src alin/src/parsers alin/src/filters alin/src/aggregators alin/src/alerters alin/src/image

# 收集所有源文件
SOURCES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
# 排除模板文件
SOURCES := $(filter-out %/atom_template.c,$(SOURCES))

# 提取节点名称
NAMES := $(basename $(notdir $(SOURCES)))

.PHONY: all clean list stream image help $(NAMES)

# 默认目标: 编译所有节点
all: $(NAMES)

# 流处理节点组
STREAM_NODES = parse_json filter_level agg_count alert_console
stream: $(STREAM_NODES)
	@echo "✅ Stream processing nodes compiled!"

# 图像处理节点组
IMAGE_NODES = decode_image encode_png passthrough filter_grayscale filter_sepia filter_invert
image: $(IMAGE_NODES)
	@echo "✅ Image processing nodes compiled!"

# MVP 节点组 (保持向后兼容)
MVP_NODES = double sum
mvp: $(MVP_NODES)

# 列出可用节点
list:
	@echo "Available nodes:"
	@echo ""
	@echo "📦 Core (MVP):"
	@for name in double sum; do \
		if [ -f "alin/src/$$name.c" ]; then \
			echo "  - $$name"; \
		fi \
	done
	@echo ""
	@echo "📥 Parsers:"
	@for f in alin/src/parsers/*.c; do \
		[ -f "$$f" ] && echo "  - $$(basename $$f .c)"; \
	done 2>/dev/null || true
	@echo ""
	@echo "🔍 Filters:"
	@for f in alin/src/filters/*.c; do \
		[ -f "$$f" ] && echo "  - $$(basename $$f .c)"; \
	done 2>/dev/null || true
	@echo ""
	@echo "📊 Aggregators:"
	@for f in alin/src/aggregators/*.c; do \
		[ -f "$$f" ] && echo "  - $$(basename $$f .c)"; \
	done 2>/dev/null || true
	@echo ""
	@echo "🚨 Alerters:"
	@for f in alin/src/alerters/*.c; do \
		[ -f "$$f" ] && echo "  - $$(basename $$f .c)"; \
	done 2>/dev/null || true

# 查找源文件的通用函数
define find_source
$(firstword $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/$(1).c)))
endef

# 通用编译规则
define compile_node
	@SRC=$$(find alin/src -name "$(1).c" 2>/dev/null | head -1); \
	if [ -z "$$SRC" ]; then \
		echo "❌ Source not found: $(1).c"; \
		exit 1; \
	fi; \
	echo "🔨 Compiling node: $(1)"; \
	echo "   Source: $$SRC"; \
	HASH=$$(md5 -q "$$SRC" | cut -c1-8); \
	OUTPUT_NAME="$(1)_$$HASH"; \
	echo "   Hash: $$HASH"; \
	echo "   Output: $(NODES_DIR)/$$OUTPUT_NAME"; \
	$(CC) $(CFLAGS) -o $(NODES_DIR)/$$OUTPUT_NAME "$$SRC"; \
	chmod +x $(NODES_DIR)/$$OUTPUT_NAME; \
	echo "✅ Compiled: $$OUTPUT_NAME"; \
	if [ -x "./scripts/alin_meta.sh" ]; then \
		./scripts/alin_meta.sh $(1) $(NODES_DIR)/$$OUTPUT_NAME 2>/dev/null || true; \
	fi
endef

# 为每个节点生成规则
$(NAMES):
	$(call compile_node,$@)

# 清理编译产物
clean:
	@echo "🧹 Cleaning..."
	@rm -f $(NODES_DIR)/*
	@rm -f $(META_DIR)/*.meta
	@rm -f alin/state/*.state
	@echo "✅ Clean complete"

# 帮助信息
help:
	@echo "ALIN Makefile - 原子节点编译系统"
	@echo ""
	@echo "使用方式:"
	@echo "  make <node>    编译指定节点 (例如: make parse_json)"
	@echo "  make all       编译所有节点"
	@echo "  make stream    编译所有流处理节点"
	@echo "  make mvp       编译 MVP 演示节点 (double, sum)"
	@echo "  make list      列出所有可用节点"
	@echo "  make clean     清理编译产物"
	@echo "  make help      显示此帮助信息"
	@echo ""
	@echo "流处理节点:"
	@echo "  parse_json     JSON 日志解析器"
	@echo "  filter_level   日志级别过滤器"
	@echo "  agg_count      事件计数聚合器"
	@echo "  alert_console  控制台告警输出"
