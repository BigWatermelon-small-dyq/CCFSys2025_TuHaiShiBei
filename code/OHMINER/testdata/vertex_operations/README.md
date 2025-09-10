# 超图顶点操作验证数据集

本数据集用于验证 `interactive_dynamic_system.cpp` 中实现的超图顶点操作功能。

## 文件说明

### 基础文件
- `hyperedges-contact-primary-school.txt`: 超边数据
- `node-labels-contact-primary-school.txt`: 顶点标签数据
- `pattern_edges.txt`: 模式图边数据
- `pattern_node_labels.txt`: 模式图顶点标签数据

### 测试文件
- `vertex_operations_commands.txt`: 基本顶点操作测试命令
- `random_vertex_operations.txt`: 随机顶点操作测试命令
- `stress_test_vertex_operations.txt`: 压力测试命令

### 脚本文件
- `run_vertex_tests.sh`: 运行所有测试的脚本
- `validate_vertex_operations.sh`: 验证结果的脚本

## 测试的顶点操作

### 1. 顶点添加 (add_vertex)
```bash
add_vertex <label>
```
- 添加一个具有指定标签的新顶点
- 返回新顶点的ID
- 自动更新匹配数量

### 2. 顶点标签修改 (set_vertex_label)
```bash
set_vertex_label <vid> <new_label>
```
- 修改指定顶点的标签
- 计算修改前后的匹配数量差异
- 更新全局匹配计数

### 3. 顶点删除 (delete_vertex)
```bash
delete_vertex <vid>
```
- 删除指定顶点
- 同时删除包含该顶点的所有超边
- 减少匹配数量

## 运行测试

```bash
# 运行所有测试
./run_vertex_tests.sh

# 或者单独运行测试
cd ../../OHMiner
./build/bin/interactive_dynamic_system < ../testdata/vertex_operations/vertex_operations_commands.txt
```

## 验证要点

1. **顶点添加**: 应该正确增加顶点数量，并更新匹配计数
2. **标签修改**: 应该正确更新标签，并计算匹配数量变化
3. **顶点删除**: 应该正确删除顶点和相关超边
4. **错误处理**: 无效操作应该被正确处理
5. **匹配更新**: 每次操作后匹配数量应该正确更新

## 预期输出格式

```
[add_vertex] v<id> label=<label>
[set_vertex_label] v<id> -> label=<new_label>
[delete_vertex] v<id>
[subgraph match] affected edges=<count>, delta=<delta>
[total] pattern matches = <count>
```

## 注意事项

- 顶点删除操作会同时删除包含该顶点的所有超边
- 标签修改操作会影响包含该顶点的超边的匹配结果
- 系统会自动维护顶点和超边之间的关联关系
