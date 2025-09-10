#!/bin/bash

# 运行顶点操作测试的脚本

echo "=== 超图顶点操作功能测试 ==="
echo ""

# 编译程序
echo "编译程序..."
cd ../../OHMiner
if [ -f "Makefile" ]; then
    make clean
    make
else
    echo "使用CMake编译..."
    if [ ! -d "build" ]; then
        mkdir -p build
    fi
    cd build
    cmake ..
    make
    cd ..
fi

if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi

echo "编译完成"
echo ""

# 运行基本测试
echo "=== 运行基本顶点操作测试 ==="
echo "测试文件: clean_vertex_operations_commands.txt"
echo ""

../../build/bin/interactive_dynamic_system < clean_vertex_operations_commands.txt

echo ""
echo "=== 运行随机顶点操作测试 ==="
echo "测试文件: random_vertex_operations.txt"
echo ""

../../build/bin/interactive_dynamic_system < random_vertex_operations.txt

echo ""
echo "=== 运行压力测试 ==="
echo "测试文件: stress_test_vertex_operations.txt"
echo ""

../../build/bin/interactive_dynamic_system < stress_test_vertex_operations.txt

echo ""
echo "=== 所有测试完成 ==="
