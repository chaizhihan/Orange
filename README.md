<div align="center">

# ⚛️ ALIN

### Atomic Logic-link Integration Network

*基于操作系统原子特性的解耦合计算范式*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Linux-blue.svg)]()
[![Language](https://img.shields.io/badge/Language-C%20%7C%20Python%20%7C%20Shell-green.svg)]()

[🎮 在线演示](https://chaizhihan.github.io/Orange/) | [📖 文档](#架构概述) | [🚀 快速开始](#快速开始)

</div>

---

## ✨ 核心理念

ALIN 将**文件系统的 Inode**作为计算的基本单元，通过**符号链接(Symlink)**实现逻辑的**原子级热切换**。

```
传统架构:  重启服务 → 中断连接 → 冷启动 → 恢复状态
ALIN架构:  ln -sf new_logic active/ → 完成! (< 1ms, 零中断)
```

### 三大定律

| 定律 | 描述 |
|------|------|
| **不可变性** | 逻辑节点一旦部署就不允许原地修改 |
| **原子性** | 路由切换必须由操作系统的原子操作完成 |
| **解耦性** | 计算逻辑不应知道自己是否正在被"热替换" |

---

## 🏗️ 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                         ALIN 架构                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐  │
│   │ Node A  │────▶│ Node B  │────▶│ Node C  │────▶│ Node D  │  │
│   │ Inode:1 │     │ Inode:2 │     │ Inode:3 │     │ Inode:4 │  │
│   └─────────┘     └────┬────┘     └─────────┘     └─────────┘  │
│                        │                                        │
│                        ▼ 热切换                                 │
│                   ┌─────────┐                                   │
│                   │ Node B' │                                   │
│                   │ Inode:5 │                                   │
│                   └─────────┘                                   │
│                                                                  │
│   [调度器] ←── stdin/stdout ──→ [状态总线]                     │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🚀 快速开始

### 安装

```bash
git clone https://github.com/chaizhihan/Orange.git
cd Orange
```

### 运行演示

```bash
# 流处理引擎演示 (日志分析)
./scripts/demo_stream.sh

# 图像处理管道演示
./scripts/demo_image.sh
```

### 查看 Web Dashboard

```bash
cd web && python3 -m http.server 8080
# 打开 http://localhost:8080
```

---

## 📦 项目结构

```
atomicLogicInodeNetwork/
├── alin/
│   ├── src/                    # 源代码
│   │   ├── parsers/            # 解析节点
│   │   ├── filters/            # 过滤节点
│   │   ├── aggregators/        # 聚合节点
│   │   ├── alerters/           # 告警节点
│   │   └── image/              # 图像处理节点
│   ├── nodes/                  # 编译后的节点
│   ├── active/                 # 当前活跃的链接
│   └── state/                  # 状态持久化
├── scripts/
│   ├── alin_run.sh             # 调度引擎
│   ├── alin_link.sh            # 热更新控制器
│   ├── alin_stream.sh          # 流处理驱动
│   └── demo_*.sh               # 演示脚本
├── web/                        # Web Dashboard
│   ├── index.html              # 流处理仪表盘
│   └── image_dashboard.html    # 图像处理仪表盘
└── docs/                       # 文档
```

---

## 🎮 演示项目

### 1. 实时流处理引擎

日志分析和监控系统，展示多节点管道和有状态处理。

```bash
# 配置管道
./scripts/alin_link.sh swap_logic 01_parse parse_json
./scripts/alin_link.sh swap_logic 02_filter filter_level
./scripts/alin_link.sh swap_logic 03_agg agg_count
./scripts/alin_link.sh swap_logic 04_alert alert_console

# 处理日志
echo '{"level":"ERROR","msg":"test"}' | ./scripts/alin_run.sh
```

**节点列表:**
- `parse_json` - JSON 日志解析
- `filter_level` - 日志级别过滤 (可热切换)
- `agg_count` - 事件计数聚合
- `alert_console` - 控制台告警

### 2. 图像处理管道

图像滤镜系统，展示滤镜热切换能力。

```bash
# 应用不同滤镜
echo '{"path":"input.jpg"}' | \
    alin/nodes/decode_image_py | \
    alin/nodes/filter_sepia_py | \
    alin/nodes/encode_png_py
```

**滤镜列表:**
- `filter_grayscale` - 灰度转换
- `filter_sepia` - 复古色调
- `filter_invert` - 颜色反转

---

## 🔥 热切换演示

```bash
# 步骤 1: 初始配置 - 只显示 ERROR
export ALIN_FILTER_LEVEL=ERROR
echo '{"level":"WARN","msg":"test"}' | ./scripts/alin_run.sh
# (无输出 - WARN 被过滤)

# 步骤 2: 热切换 - 显示 WARN 及以上
export ALIN_FILTER_LEVEL=WARN
echo '{"level":"WARN","msg":"test"}' | ./scripts/alin_run.sh
# (显示告警 - 无需重启!)
```

---

## 🌐 Web Dashboard

<div align="center">

### 流处理仪表盘

实时监控日志流、事件统计和管道拓扑。

### 图像处理仪表盘

实时预览滤镜效果，一键热切换。

</div>

---

## 📖 文档

- [架构设计](docs/ARCHITECTURE.md)
- [API 参考](docs/API.md)
- [开发指南](docs/CONTRIBUTING.md)

---

## 🤝 贡献

欢迎贡献! 请查看 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详情。

---

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE)

---

<div align="center">

**ALIN** - *Where Logic Flows Through Links*

Made with ⚛️ by [Your Name]

</div>
