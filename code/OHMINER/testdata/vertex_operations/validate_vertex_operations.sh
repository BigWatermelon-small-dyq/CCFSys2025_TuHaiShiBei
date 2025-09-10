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
