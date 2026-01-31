# 🛠 ALIN 环境构建指令提纲 (Prompt for AI Assistant)

本提纲旨在指导 AI（如 Claude Code 或其他 Agent）从零构建一个符合 ALIN 规范的开发环境。

## 1. 项目基础结构初始化 (The Inode Map)

要求初始化以下目录结构，这决定了 ALIN 的物理拓扑：

*   `/alin/nodes`: 存储所有原子的二进制 Inode（C/Rust 编译产物）。
*   `/alin/active`: 存储当前生效的逻辑链接（Dentry）。
*   `/alin/flows`: 存储编排逻辑的 Shell 脚本。
*   `/alin/data`: 存储输入输出流。

## 2. 定义“原子节点”规范 (The Atomic Spec)

生成一个简单的 C 语言模板，确保每个函数都是原子的：

*   **输入/输出**: 强制通过 `stdin` 和 `stdout` 进行流式传输。
*   **独立编译**: 编写一个 Makefile，支持对单个 `.c` 文件进行独立编译，并将其移动到 `/alin/nodes` 下，文件名格式为 `[name]_[hash]`。

## 3. 构建“路径寻址”调度引擎 (The Shell Orchestrator)

编写核心脚本 `alin_run.sh`，实现以下功能：

*   **拓扑扫描**: 利用 `ls -i` 读取 `/alin/active` 下的 Inode 列表。
*   **动态管道组装**: 根据文件夹内 Dentry 的字母顺序（或命名编号），自动构建类似 `node1 | node2 | node3` 的 Linux 管道链。
*   **状态透明化**: 执行时输出当前路径指向的具体 Inode 编号，确保 AI 可读。

## 4. 实现“热更新”机制 (Atomic Linkage)

演示如何不停止系统进行逻辑切换：

*   编写脚本 `alin_link.sh`，封装 `ln -sf` 命令。
*   **场景模拟**: 将 `/alin/active/01_process` 从指向 `node_v1` 切换到 `node_v2`。

## 5. 编写 AI 快速索引描述 (Metadata Indexing)

为每个 Inode 自动生成一个 `.meta` 文本文件：

*   **包含**: 函数功能描述、输入格式、输出格式。
*   **目的**: 让 AI 后续阅读代码时，只需读取 `.meta` 就能理解整个拓扑，无需解析二进制或庞大的源码。

---

## 🚀 最小可行性演示 (MVP Case: 数据加倍与求和)

**指令示例**:
> "请按照 ALIN 架构为我建立一个测试环境：
> 1. 用 C 语言写两个原子函数：`double` (数值翻倍) 和 `sum` (求和)。
> 2. 将它们编译成独立的二进制文件并放入 `/alin/nodes`。
> 3. 在 `/alin/active` 中建立软链接 `01_dbl` 和 `02_sum`。
> 4. 编写一个 Shell 脚本，通过遍历 `/alin/active` 目录，将输入数据 `[1, 2, 3]` 流式通过这些 Inode。
> 5. 演示如何通过修改 `/alin/active` 的链接，将逻辑改为 `sum` 后再 `double`。"
