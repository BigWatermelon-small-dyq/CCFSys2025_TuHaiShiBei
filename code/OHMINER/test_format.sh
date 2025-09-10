#!/bin/bash

# 测试命令格式的脚本

echo "=== 测试命令格式 ==="

# 测试添加超边格式
test_line="11,22,33,44"
vertex_count=$(echo "$test_line" | tr ',' '\n' | wc -l)
vertices_space=$(echo "$test_line" | tr ',' ' ')
echo "原始格式: $test_line"
echo "顶点个数: $vertex_count"
echo "空格分隔: $vertices_space"
echo "add_edge命令: add_edge $vertex_count $vertices_space"

echo ""

# 测试修改超边格式
test_line="100,162,163"
vertex_count=$(echo "$test_line" | tr ',' '\n' | wc -l)
vertices_space=$(echo "$test_line" | tr ',' ' ')
echo "原始格式: $test_line"
echo "顶点个数: $vertex_count"
echo "空格分隔: $vertices_space"
echo "modify_edge命令: modify_edge 0 $vertex_count $vertices_space"

echo ""
echo "测试完成!"
