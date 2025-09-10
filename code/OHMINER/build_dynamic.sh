#!/bin/bash

echo "OHMiner 动态超图匹配系统构建脚本"
echo "=================================="

# 检查是否在正确的目录
if [ ! -f "CMakeLists.txt" ]; then
    echo "错误: 请在OHMiner根目录下运行此脚本"
    exit 1
fi

# 创建构建目录
echo "创建构建目录..."
if [ -d "build" ]; then
    echo "构建目录已存在，清理中..."
    rm -rf build
fi
mkdir build
cd build

# 运行CMake配置
echo "配置CMake..."
cmake .. || {
    echo "CMake配置失败"
    exit 1
}

# 编译
echo "编译系统..."
make -j$(nproc) || {
    echo "编译失败"
    exit 1
}

echo "编译完成！"

# 检查可执行文件
echo "检查生成的可执行文件..."
if [ -f "test/dynamic_hypergraph_test" ]; then
    echo "✓ 动态超图测试程序已生成"
else
    echo "✗ 动态超图测试程序生成失败"
fi

if [ -f "libHyperMining.so" ]; then
    echo "✓ 动态超图库已生成"
else
    echo "✗ 动态超图库生成失败"
fi

echo ""
echo "构建完成！"
echo ""
echo "运行测试:"
echo "  cd build"
echo "  ./test/dynamic_hypergraph_test"
echo ""
echo "运行原有测试:"
echo "  ./test/overlap_match"
echo "  ./test/overlap_match_parallel"
echo "  ./test/random_sample"





