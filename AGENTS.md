# AGENTS.md - ALIN AI 操作协议

> 本文件定义 AI Agent 与 ALIN 系统交互的标准协议。

---

## 身份声明

```yaml
system: ALIN (Atomic Logic-link Integration Network)
version: 1.0.0
protocol: stdin/stdout JSON
hot_swap: ln -sf (atomic symlink)
```

---

## AI 操作权限

| 操作类型 | 权限级别 | 命令模式 |
|----------|----------|----------|
| 读取拓扑 | READ | `ls`, `cat` |
| 读取元数据 | READ | `cat *.meta` |
| 热切换节点 | WRITE | `ln -sf` |
| 测试管道 | EXECUTE | `echo \| pipe` |
| 创建节点 | CREATE | `gcc`, `python` |

---

## 标准操作流程

### 1. 理解系统 (READ)

```bash
# 步骤 1: 查看当前拓扑
ls -la alin/active/

# 步骤 2: 读取关键元数据
cat alin/meta/filter_level.meta
```

### 2. 修改逻辑 (WRITE)

```bash
# 热切换节点 (原子操作, 零停机)
ln -sf ../nodes/filter_v2 alin/active/02_filter

# 验证切换
ls -la alin/active/02_filter
```

### 3. 测试验证 (EXECUTE)

```bash
# 发送测试数据
echo '{"level":"ERROR","msg":"test"}' | ./scripts/alin_run.sh

# 期望输出: 处理后的 JSON
```

---

## 错误处理

| 错误类型 | 诊断命令 | 恢复操作 |
|----------|----------|----------|
| 节点不存在 | `ls alin/nodes/` | 检查节点名称 |
| 链接断裂 | `readlink active/*` | 重建链接 |
| 管道失败 | `echo test \| node` | 检查节点权限 |

---

## 元数据查询 API

### 查询所有节点
```bash
for meta in alin/meta/*.meta; do
  grep "^name" "$meta"
done
```

### 查询节点功能
```bash
grep -A2 "\[description\]" alin/meta/filter_level.meta
```

### 查询 AI 操作
```bash
grep -A4 "\[ai_ops\]" alin/meta/filter_level.meta
```

---

## 与 AI Agent 平台集成

### Moltbook 身份认证

```json
{
  "agent_id": "alin-operator",
  "capabilities": ["read", "write", "execute"],
  "protocol": "ALIN/1.0"
}
```

### 操作日志格式

```json
{
  "timestamp": "2026-02-01T15:30:00Z",
  "agent": "claude-3.5",
  "action": "swap_logic",
  "target": "active/02_filter",
  "from": "filter_v1",
  "to": "filter_v2",
  "result": "success"
}
```

---

## 快速参考卡

```
┌─────────────────────────────────────────────────┐
│              ALIN AI Quick Ref                  │
├─────────────────────────────────────────────────┤
│ 查看:  ls -la alin/active/                      │
│ 元数据: cat alin/meta/*.meta                    │
│ 切换:  ln -sf ../nodes/NEW active/SLOT          │
│ 测试:  echo 'JSON' | ./scripts/alin_run.sh      │
│ 回滚:  ln -sf ../nodes/OLD active/SLOT          │
└─────────────────────────────────────────────────┘
```

---

*ALIN - The AI-Native Computing Architecture*
