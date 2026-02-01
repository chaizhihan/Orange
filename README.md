<div align="center">

# ⚛️ ALIN

### Atomic Logic Inode Network

*基于操作系统原子特性的解耦合计算范式*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Linux-blue.svg)]()
[![Language](https://img.shields.io/badge/Language-C%20%7C%20Python%20%7C%20Shell-green.svg)]()

[🎮 在线演示](https://chaizhihan.github.io/Orange/) | [📖 文档](#架构概述) | [🚀 快速开始](#快速开始)

</div>

---

## 📖 架构说明书 (Architecture Specification)

ALIN 架构将复杂的实时计算任务拆解为三个抽象层，通过原子化路径路由实现逻辑与执行的解耦合。

### 1. 核心定义 (Core Concepts)

*   **逻辑节点 (Logic Nodes)**: 存储在物理介质上的不可变计算单元（脚本、二进制、或容器镜像）。
*   **原子路由 (Atomic Router)**: 利用操作系统文件系统的符号链接 (Symlink) 或硬链接实现的“逻辑指针”。
*   **状态总线 (State Bus)**: 跨越逻辑切换周期、持续存在的内存空间或数据流通道。

---

### 2. 系统拓扑图 (Topology)

在 ALIN 架构中，数据不是流向特定的“代码文件”，而是流向一个虚路径 (Virtual Path)。

**数据流向**: 输入 $I$ $\rightarrow$ 虚路径端口 $P$ (当前指向 Node A) $\rightarrow$ 处理结果 $O$ + 持久状态 $S$。

**切换过程**: 外部指令 $\rightarrow$ 修改 $P$ 的指向 $\rightarrow$ $P$ 瞬间指向 Node B（Node A 的进程无感完成最后一次计算后销毁，新进程无缝接手）。

```
┌─────────────────────────────────────────────────────────────────┐
│                         ALIN 拓扑                               │
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

### 3. 通用接口定义 (Interface Definitions)

#### A. 节点执行接口 (Node Execution Interface)

任何 ALIN 兼容节点必须实现以下输入输出规范：

```plaintext
Input:  (data_payload, context_state)
Output: (result_payload, next_state)
```

*   **data_payload**: 当前迭代的输入数据。
*   **context_state**: 从上一个节点或上一次迭代继承的“记忆”。

#### B. 路由器控制接口 (Routing Control Interface)

这是 ALIN 架构最核心的“热替换”接口：

| 接口方法 | 参数 | 说明 |
| :--- | :--- | :--- |
| `swap_logic` | `path_alias`, `target_node_id` | 利用 `ln -sf` 或 `rename()` 原子性地修改路由指向。 |
| `health_check` | `path_alias` | 验证当前路由指向的节点是否处于“就绪”状态。 |
| `rollback` | `path_alias` | 发生异常时，瞬间切回上一个已知的稳定 Inode 指向。 |

#### C. 状态持久化接口 (State Persistence Interface)

1.  **Freeze**: 逻辑切换触发时，锁定当前状态。
2.  **Snapshot**: 将状态序列化或映射到共享内存。
3.  **Restore**: 新逻辑节点启动后，首先从公共区域拉取 Snapshot。

---

### 4. 架构的三大定律 (The Three Laws of ALIN)

1.  **不可变性定律**: 逻辑节点一旦部署就不允许原地修改。所有的更新必须通过“创建新节点 + 修改路由”完成。
2.  **原子性定律**: 路由切换必须由操作系统的原子操作完成，确保系统中不存在“中间态”。
3.  **解耦定律**: 计算逻辑不应知道自己是否正在被“热替换”，也不应关心是哪个工具把它运行起来的。

---

## ✨ 核心理念 (Philosophy)

ALIN 将**文件系统的 Inode**作为计算的基本单元，通过**符号链接(Symlink)**实现逻辑的**原子级热切换**。

```
传统架构:  重启服务 → 中断连接 → 冷启动 → 恢复状态
ALIN架构:  ln -sf new_logic active/ → 完成! (< 1ms, 零中断)
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

GitHub Pages: [https://chaizhihan.github.io/Orange/](https://chaizhihan.github.io/Orange/)

本地运行:
```bash
cd docs && python3 -m http.server 8080
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

- [架构说明书 (Specification)](docs/SPECIFICATION.md)
- [环境构建指令提纲 (Agent Prompt Guide)](docs/PROMPT_GUIDE.md)
- [架构设计深度解析 (Architecture Detail)](docs/ARCHITECTURE.md)
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

Made with ⚛️ by ChaiZhiHan

</div>
