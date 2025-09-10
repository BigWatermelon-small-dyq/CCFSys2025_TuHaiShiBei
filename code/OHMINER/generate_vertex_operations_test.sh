#!/bin/bash

# 生成超图中点的增加、修改、删除功能验证数据集
# 基于 interactive_dynamic_system.cpp 中的命令解析

set -e

# 配置参数
BASE_DATASET="../hypergraphdataset/contact-primary-school"
OUTPUT_DIR="testdata/vertex_operations"
COMMANDS_FILE="vertex_operations_commands.txt"
HYPEREDGES_FILE="hyperedges-contact-primary-school.txt"
NODE_LABELS_FILE="node-labels-contact-primary-school.txt"
PATTERN_EDGES_FILE="../pattern_edges.txt"
PATTERN_NODE_LABELS_FILE="../pattern_node_labels.txt"

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

echo "=== 生成超图顶点操作验证数据集 ==="
echo "基础数据集: $BASE_DATASET"
echo "输出目录: $OUTPUT_DIR"
echo "命令文件: $COMMANDS_FILE"
echo ""

# 复制基础数据集文件
echo "复制基础数据集文件..."
cp "$BASE_DATASET/$HYPEREDGES_FILE" "$OUTPUT_DIR/"
cp "$BASE_DATASET/$NODE_LABELS_FILE" "$OUTPUT_DIR/"
cp "$PATTERN_EDGES_FILE" "$OUTPUT_DIR/"
cp "$PATTERN_NODE_LABELS_FILE" "$OUTPUT_DIR/"

# 验证复制的文件
echo "验证复制的文件..."
echo "超边文件行数: $(wc -l < "$OUTPUT_DIR/$HYPEREDGES_FILE")"
echo "节点标签文件行数: $(wc -l < "$OUTPUT_DIR/$NODE_LABELS_FILE")"
echo "模式边文件行数: $(wc -l < "$OUTPUT_DIR/pattern_edges.txt")"
echo "模式节点标签文件行数: $(wc -l < "$OUTPUT_DIR/pattern_node_labels.txt")"

echo "基础文件复制完成"
echo ""

# 生成顶点操作命令序列
echo "生成顶点操作命令序列..."

cat > "$OUTPUT_DIR/$COMMANDS_FILE" << 'EOF'
# 超图顶点操作验证命令序列
# 基于 interactive_dynamic_system.cpp 的命令解析

# 1. 加载超图和模式图
load_hg hyperedges-contact-primary-school.txt node-labels-contact-primary-school.txt
load_pattern pattern_edges.txt pattern_node_labels.txt

# 2. 初始匹配
match

# 3. 顶点增加操作测试
echo "=== 开始顶点增加操作测试 ==="
add_vertex 1
add_vertex 2
add_vertex 3
add_vertex 1
add_vertex 2

# 4. 顶点标签修改操作测试
echo "=== 开始顶点标签修改操作测试 ==="
# 修改一些现有顶点的标签
set_vertex_label 0 2
set_vertex_label 1 3
set_vertex_label 2 1
set_vertex_label 5 2
set_vertex_label 10 3

# 5. 顶点删除操作测试
echo "=== 开始顶点删除操作测试 ==="
# 删除一些顶点（注意：删除顶点会同时删除相关的超边）
delete_vertex 0
delete_vertex 1
delete_vertex 2

# 6. 混合操作测试
echo "=== 开始混合操作测试 ==="
# 添加新顶点
add_vertex 1
add_vertex 2
add_vertex 3

# 修改新添加顶点的标签
set_vertex_label 0 2
set_vertex_label 1 3

# 再次添加顶点
add_vertex 1
add_vertex 2

# 7. 边界情况测试
echo "=== 开始边界情况测试 ==="
# 尝试修改不存在的顶点标签
set_vertex_label 999 1

# 尝试删除不存在的顶点
delete_vertex 999

# 8. 最终匹配验证
echo "=== 最终匹配验证 ==="
match

# 9. 退出
quit
EOF

echo "顶点操作命令序列生成完成"
echo ""

# 生成随机顶点操作测试
echo "生成随机顶点操作测试..."

cat > "$OUTPUT_DIR/random_vertex_operations.txt" << 'EOF'
# 随机顶点操作测试
# 基于 contact-primary-school 数据集

load_hg hyperedges-contact-primary-school.txt node-labels-contact-primary-school.txt
load_pattern pattern_edges.txt pattern_node_labels.txt
match

# 随机顶点操作序列
add_vertex 1
add_vertex 2
add_vertex 3
set_vertex_label 0 2
set_vertex_label 1 3
add_vertex 1
set_vertex_label 2 1
add_vertex 2
set_vertex_label 3 2
add_vertex 3
set_vertex_label 4 3
delete_vertex 0
add_vertex 1
set_vertex_label 1 2
add_vertex 2
set_vertex_label 2 1
delete_vertex 1
add_vertex 3
set_vertex_label 3 2
add_vertex 1
set_vertex_label 4 3
add_vertex 2
set_vertex_label 5 1
delete_vertex 2
add_vertex 3
set_vertex_label 6 2
add_vertex 1
set_vertex_label 7 3
add_vertex 2
set_vertex_label 8 1
delete_vertex 3
add_vertex 3
set_vertex_label 9 2
add_vertex 1
set_vertex_label 10 3
match
quit
EOF

echo "随机顶点操作测试生成完成"
echo ""

# 生成压力测试
echo "生成压力测试..."

cat > "$OUTPUT_DIR/stress_test_vertex_operations.txt" << 'EOF'
# 顶点操作压力测试
# 大量顶点操作以测试系统稳定性

load_hg hyperedges-contact-primary-school.txt node-labels-contact-primary-school.txt
load_pattern pattern_edges.txt pattern_node_labels.txt
match

# 大量顶点添加
add_vertex 1
add_vertex 2
add_vertex 3
add_vertex 1
add_vertex 2
add_vertex 3
add_vertex 1
add_vertex 2
add_vertex 3
add_vertex 1
add_vertex 2
add_vertex 3
add_vertex 1
add_vertex 2
add_vertex 3

# 大量标签修改
set_vertex_label 0 2
set_vertex_label 1 3
set_vertex_label 2 1
set_vertex_label 3 2
set_vertex_label 4 3
set_vertex_label 5 1
set_vertex_label 6 2
set_vertex_label 7 3
set_vertex_label 8 1
set_vertex_label 9 2
set_vertex_label 10 3
set_vertex_label 11 1
set_vertex_label 12 2
set_vertex_label 13 3
set_vertex_label 14 1

# 大量顶点删除
delete_vertex 0
delete_vertex 1
delete_vertex 2
delete_vertex 3
delete_vertex 4
delete_vertex 5
delete_vertex 6
delete_vertex 7
delete_vertex 8
delete_vertex 9
delete_vertex 10
delete_vertex 11
delete_vertex 12
delete_vertex 13
delete_vertex 14

# 最终验证
match
quit
EOF

echo "压力测试生成完成"
echo ""

# 生成测试脚本
echo "生成测试脚本..."

cat > "$OUTPUT_DIR/run_vertex_tests.sh" << 'EOF'
#!/bin/bash

# 运行顶点操作测试的脚本

echo "=== 超图顶点操作功能测试 ==="
echo ""

# 编译程序
echo "编译程序..."
cd ../../OHMiner
make clean
make

if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi

echo "编译完成"
echo ""

# 运行基本测试
echo "=== 运行基本顶点操作测试 ==="
echo "测试文件: vertex_operations_commands.txt"
echo ""

./build/bin/interactive_dynamic_system < ../testdata/vertex_operations/vertex_operations_commands.txt

echo ""
echo "=== 运行随机顶点操作测试 ==="
echo "测试文件: random_vertex_operations.txt"
echo ""

./build/bin/interactive_dynamic_system < ../testdata/vertex_operations/random_vertex_operations.txt

echo ""
echo "=== 运行压力测试 ==="
echo "测试文件: stress_test_vertex_operations.txt"
echo ""

./build/bin/interactive_dynamic_system < ../testdata/vertex_operations/stress_test_vertex_operations.txt

echo ""
echo "=== 所有测试完成 ==="
EOF

chmod +x "$OUTPUT_DIR/run_vertex_tests.sh"

echo "测试脚本生成完成"
echo ""

# 生成验证脚本
echo "生成验证脚本..."

cat > "$OUTPUT_DIR/validate_vertex_operations.sh" << 'EOF'
#!/bin/bash

# 验证顶点操作结果的脚本

echo "=== 验证顶点操作结果 ==="
echo ""

# 检查输出文件
echo "检查输出文件..."
ls -la

echo ""
echo "=== 验证要点 ==="
echo "1. 顶点添加操作应该正确增加顶点数量"
echo "2. 顶点标签修改应该正确更新标签"
echo "3. 顶点删除应该正确删除顶点和相关超边"
echo "4. 每次操作后匹配数量应该正确更新"
echo "5. 错误操作应该被正确处理"
echo ""

echo "=== 预期输出格式 ==="
echo "[add_vertex] v<id> label=<label>"
echo "[set_vertex_label] v<id> -> label=<new_label>"
echo "[delete_vertex] v<id>"
echo "[subgraph match] affected edges=<count>, delta=<delta>"
echo "[total] pattern matches = <count>"
echo ""

echo "验证脚本完成"
EOF

chmod +x "$OUTPUT_DIR/validate_vertex_operations.sh"

echo "验证脚本生成完成"
echo ""

# 生成README
echo "生成README文档..."

cat > "$OUTPUT_DIR/README.md" << 'EOF'
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
EOF

echo "README文档生成完成"
echo ""

echo "=== 数据集生成完成 ==="
echo "输出目录: $OUTPUT_DIR"
echo "包含文件:"
ls -la "$OUTPUT_DIR"
echo ""
echo "运行测试:"
echo "cd $OUTPUT_DIR && ./run_vertex_tests.sh"
echo ""
echo "验证结果:"
echo "cd $OUTPUT_DIR && ./validate_vertex_operations.sh"
