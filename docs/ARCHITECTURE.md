# ALIN 架构设计文档

## 概述

ALIN (Atomic Logic-link Integration Network) 是一种基于操作系统原子特性的解耦合计算范式。它将**文件系统的 Inode**作为计算的基本单元，通过**符号链接(Symlink)**实现逻辑的**原子级热切换**。

## 核心概念

### 1. 逻辑节点 (Logic Node)

逻辑节点是 ALIN 的基本计算单元。每个节点：

- 是一个独立的可执行文件
- 有唯一的 Inode 编号
- 从 stdin 读取输入
- 向 stdout 写出输出
- 遵循"处理一行输入，产生一行输出"的原则

```
[节点示例]
┌─────────────────────────────┐
│  filter_level_abc123        │
│  Inode: 7335321             │
├─────────────────────────────┤
│  stdin  → 处理逻辑 → stdout │
│           ↓                 │
│        环境变量配置          │
└─────────────────────────────┘
```

### 2. 原子路由 (Atomic Router)

原子路由是将逻辑名称映射到物理节点的机制：

```
alin/active/
├── 01_parse  → ../nodes/parse_json_abc123
├── 02_filter → ../nodes/filter_level_def456
└── 03_agg    → ../nodes/agg_count_ghi789
```

**热切换原理：**
```bash
# 原子操作：ln -sf 是操作系统保证的原子操作
ln -sf ../nodes/filter_level_NEW_HASH alin/active/02_filter
```

### 3. 状态总线 (State Bus)

状态总线是数据在节点之间流动的通道：

```
[Input] → [Node A] → [Node B] → [Node C] → [Output]
             │           │           │
             ▼           ▼           ▼
         state/a     state/b     state/c
```

每个节点可以拥有自己的状态文件，实现有状态处理。

## 三大定律

### 第一定律：不可变性

> 逻辑节点一旦部署就不允许原地修改

节点是不可变的（Immutable）。要更新逻辑，必须编译新版本并创建新的 Inode。

### 第二定律：原子性

> 路由切换必须由操作系统的原子操作完成

利用操作系统提供的 `rename()` 系统调用实现原子切换，确保不会出现中间状态。

### 第三定律：解耦性

> 计算逻辑不应知道自己是否正在被"热替换"

节点只关心自己的输入和输出，不需要知道调度器的存在。

## 节点命名规范

```
[name]_[hash]

示例：
- parse_json_abc12345
- filter_level_def67890
- agg_count_ghi11111

其中：
- name: 节点功能名称
- hash: 源代码 MD5 的前8位
```

## 数据协议

### JSON 事件格式

```json
{
  "_type": "log",
  "level": "ERROR",
  "message": "...",
  "timestamp": 1234567890,
  "_raw": { /* 原始输入 */ },
  "_agg": { /* 聚合数据 */ }
}
```

### 图像数据格式

```json
{
  "_type": "image",
  "width": 200,
  "height": 200,
  "format": "ppm",
  "ppm": "<base64 encoded PPM data>"
}
```

## 目录结构

```
alin/
├── src/           # 源代码
│   ├── parsers/   # 解析节点
│   ├── filters/   # 过滤节点
│   ├── aggregators/ # 聚合节点
│   ├── alerters/  # 告警节点
│   └── image/     # 图像处理节点
├── nodes/         # 编译后的可执行文件
├── active/        # 当前活跃的符号链接
├── state/         # 状态持久化
└── meta/          # 节点元数据
```

## 使用示例

### 配置管道

```bash
./scripts/alin_link.sh swap_logic 01_parse parse_json
./scripts/alin_link.sh swap_logic 02_filter filter_level
./scripts/alin_link.sh swap_logic 03_agg agg_count
./scripts/alin_link.sh swap_logic 04_alert alert_console
```

### 执行处理

```bash
echo '{"level":"ERROR","msg":"test"}' | ./scripts/alin_run.sh
```

### 热切换

```bash
# 运行时切换过滤级别
export ALIN_FILTER_LEVEL=DEBUG
./scripts/alin_link.sh swap_logic 02_filter filter_level
```

## 优势

1. **零停机维护** - 热切换不影响正在处理的数据
2. **可追溯性** - Inode 提供精确的执行路径追踪
3. **简单可靠** - 基于操作系统原语，无需复杂的分布式协调
4. **语言无关** - 节点可以用任何语言实现
5. **易于测试** - 每个节点可以独立测试

---

*ALIN - Where Logic Flows Through Links*
