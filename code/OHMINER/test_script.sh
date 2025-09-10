#!/bin/bash

# 简化的测试脚本，只处理addedges文件夹

echo "=== 测试脚本启动 ==="

# 获取原始超图文件作为基准
ORIGINAL_FILE="../hypergraphdataset/contact-primary-school/hyperedges-contact-primary-school.txt"
FOLDER="../testdata/addedges"
HYPEREDGE_FILE="$FOLDER/hyperedges-contact-primary-school.txt"

echo "原始文件: $ORIGINAL_FILE"
echo "测试文件夹: $FOLDER"
echo "超边文件: $HYPEREDGE_FILE"

if [ ! -f "$HYPEREDGE_FILE" ]; then
    echo "错误: 找不到超边文件"
    exit 1
fi

echo ""
echo "执行添加超边操作..."
echo "新增的超边:"

# 比较文件，找出新增的超边
comm -13 <(sort "$ORIGINAL_FILE") <(sort "$HYPEREDGE_FILE") | while read -r line; do
    if [[ -n "$line" && ! "$line" =~ ^[[:space:]]*$ ]]; then
        echo "add_edge $line"
    fi
done

echo ""
echo "测试完成!"
