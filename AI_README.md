# ALIN - AI Agent 快速理解指南

> 本文件专为 AI Agent 设计，帮助你在 **500 tokens 内**理解并操作 ALIN 系统。

---

## 系统拓扑 (一句话理解)

```
stdin → [01_parse] → [02_filter] → [03_agg] → [04_alert] → stdout
         ↑            ↑             ↑           ↑
         └── 每个节点都是独立 Inode，可热切换 ──┘
```

---

## 目录结构

```
alin/
├── nodes/      # 二进制节点 (Inode)
├── active/     # 当前生效的软链接 (Dentry)
├── meta/       # 节点元数据 (.meta 文件) ← AI 读这里
└── src/        # 源代码
```

---

## 常用操作 (复制即用)

### 查看当前拓扑
```bash
ls -la alin/active/
```

### 热切换节点
```bash
ln -sf ../nodes/filter_v2 alin/active/02_filter
```

### 读取节点元数据
```bash
cat alin/meta/filter_level.meta
```

### 运行管道
```bash
echo '{"level":"ERROR","msg":"test"}' | ./scripts/alin_run.sh
```

---

## .meta 文件格式

```ini
[node]
name = filter_level
hash = abc12345

[description]
过滤日志级别

[interface]
input  = JSON {"level": "...", "msg": "..."}
output = 通过过滤的事件

[ai_ops]
enable  = ln -sf filter_level active/02_filter
disable = rm active/02_filter
config  = export ALIN_FILTER_LEVEL=WARN
```

---

## AI 操作清单

| 任务 | 命令 |
|------|------|
| 查看拓扑 | `ls -la alin/active/` |
| 切换节点 | `ln -sf ../nodes/NEW active/SLOT` |
| 查看元数据 | `cat alin/meta/*.meta` |
| 测试管道 | `echo 'JSON' \| ./scripts/alin_run.sh` |
| 回滚版本 | `ln -sf ../nodes/OLD active/SLOT` |

---

## 为什么 ALIN 对 AI 友好?

1. **元数据优先**: 读 .meta 即可理解节点，无需解析源码
2. **原子操作**: `ln -sf` 一条命令完成热切换
3. **标准接口**: 所有节点 stdin/stdout JSON 协议
4. **拓扑透明**: 文件系统即架构，`ls` 即可观测

---

*ALIN - Making AI Operations Atomic*
