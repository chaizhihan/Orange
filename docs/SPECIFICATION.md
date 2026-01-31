# ALIN - Atomic Logic Inode Network 架构说明书

## 1. 核心定义 (Core Concepts)

ALin 架构将复杂的实时计算任务拆解为三个抽象层，通过原子化路径路由实现逻辑与执行的解耦合。

*   **逻辑节点 (Logic Nodes)**: 存储在物理介质上的不可变计算单元（脚本、二进制、或容器镜像）。
*   **原子路由 (Atomic Router)**: 利用操作系统文件系统的符号链接 (Symlink) 或硬链接实现的“逻辑指针”。
*   **状态总线 (State Bus)**: 跨越逻辑切换周期、持续存在的内存空间或数据流通道。

## 2. 系统拓扑图 (Topology)

在 ALin 架构中，数据不是流向特定的“代码文件”，而是流向一个虚路径 (Virtual Path)。

**数据流向**: 输入 $I$ $\rightarrow$ 虚路径端口 $P$ (当前指向 Node A) $\rightarrow$ 处理结果 $O$ + 持久状态 $S$。

**切换过程**: 外部指令 $\rightarrow$ 修改 $P$ 的指向 $\rightarrow$ $P$ 瞬间指向 Node B（Node A 的进程无感完成最后一次计算后销毁，新进程无缝接手）。

## 3. 通用接口定义 (Interface Definitions)

为了让 ALin 架构支持跨语言和跨环境，定义了一套通用的标准接口。

### A. 节点执行接口 (Node Execution Interface)

任何 ALin 兼容节点必须实现以下输入输出规范：

```plaintext
Input:  (data_payload, context_state)
Output: (result_payload, next_state)
```

*   **data_payload**: 当前迭代的输入数据。
*   **context_state**: 从上一个节点或上一次迭代继承的“记忆”。

### B. 路由器控制接口 (Routing Control Interface)

这是 ALin 架构最核心的“热替换”接口，通常由一个外部控制器（Controller）调用：

| 接口方法 | 参数 | 说明 |
| :--- | :--- | :--- |
| `swap_logic` | `path_alias`, `target_node_id` | 利用 `ln -sf` 或 `rename()` 原子性地修改路由指向。 |
| `health_check` | `path_alias` | 验证当前路由指向的节点是否处于“就绪”状态。 |
| `rollback` | `path_alias` | 发生异常时，瞬间切回上一个已知的稳定 Inode 指向。 |

### C. 状态持久化接口 (State Persistence Interface)

为了保证热替换时不丢数据，ALin 定义了状态的“移交协议”：

1.  **Freeze**: 逻辑切换触发时，锁定当前状态。
2.  **Snapshot**: 将状态序列化或映射到公共共享内存。
3.  **Restore**: 新逻辑节点启动后，首先从公共区域拉取 Snapshot。

## 4. 架构的三大定律 (The Three Laws of ALin)

1.  **不可变性定律**: 逻辑节点一旦部署就不允许原地修改。所有的更新必须通过“创建新节点 + 修改路由”完成。
2.  **原子性定律**: 路由切换必须由操作系统的原子操作完成，确保系统中不存在“中间态”。
3.  **解耦定律**: 计算逻辑不应知道自己是否正在被“热替换”，也不应关心是哪个工具把它运行起来的。

## 5. 架构优势总结

*   **无感升级**: 通过 `ln -sf` 的原子特性，在 Iteration $N$ 和 $N+1$ 之间完成逻辑漂移。
*   **多语言兼容**: 只要符合文件系统路径读取规则，Node 可以是任何语言编写的。
*   **天然容错**: 如果 Node B 挂了，只需要把软链接切回 Node A，这就是最快速度的“秒级回滚”。
