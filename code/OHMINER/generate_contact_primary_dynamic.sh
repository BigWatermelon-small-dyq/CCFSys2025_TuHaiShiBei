#!/bin/bash

# 说明：
# 基于 contact-primary-school 超图数据集，按 70% 作为初始原图，
# 剩余 30% 的超边先执行插入(add_edge)，随后对同一批边执行修改(modify_edge)，最后删除(delete_edge)。
# 会生成：
#   1) 初始 70% 超边文件：OHMiner/testdata/generated/initial_hyperedges.txt
#   2) 初始化命令文件：  OHMiner/testdata/generated/initial_commands.txt   (仅 load_hg + load_pattern)
#   3) 动态操作命令文件：OHMiner/testdata/generated/dynamic_ops.txt       (仅 add/modify/delete + quit)
#   4) 合并总命令文件：  OHMiner/testdata/generated/commands_all.txt      (2)+(3)
# 可直接用如下命令运行系统：
#   cd /home/w1nner/learn/OHMiner/OHMiner && ./build/bin/interactive_dynamic_system < testdata/generated/commands_all.txt

set -euo pipefail

# 数据与输出路径（使用绝对路径，便于直接运行）
ROOT_DIR="/home/w1nner/learn/OHMiner"
OHM_DIR="$ROOT_DIR/OHMiner"
HG_DIR="$ROOT_DIR/hypergraphdataset/contact-primary-school"

ORIGINAL_EDGES="$HG_DIR/hyperedges-contact-primary-school.txt"
NODE_LABELS="$HG_DIR/node-labels-contact-primary-school.txt"
PATTERN_EDGES="$ROOT_DIR/pattern_edges.txt"
PATTERN_NODE_LABELS="$ROOT_DIR/pattern_node_labels.txt"

GEN_DIR="$OHM_DIR/testdata/generated"
INITIAL_EDGES="$GEN_DIR/initial_hyperedges.txt"
INITIAL_COMMANDS="$GEN_DIR/initial_commands.txt"
DYNAMIC_OPS="$GEN_DIR/dynamic_ops.txt"
COMMANDS_ALL="$GEN_DIR/commands_all.txt"

mkdir -p "$GEN_DIR"

if [ ! -f "$ORIGINAL_EDGES" ]; then
    echo "错误：未找到超边文件：$ORIGINAL_EDGES" >&2
    exit 1
fi
if [ ! -f "$NODE_LABELS" ]; then
    echo "错误：未找到节点标签文件：$NODE_LABELS" >&2
    exit 1
fi
if [ ! -f "$PATTERN_EDGES" ] || [ ! -f "$PATTERN_NODE_LABELS" ]; then
    echo "错误：未找到模式文件：$PATTERN_EDGES 或 $PATTERN_NODE_LABELS" >&2
    exit 1
fi

# 统计总行数并计算 70% 分割点（向下取整）
TOTAL=$(wc -l < "$ORIGINAL_EDGES")
if [ "$TOTAL" -le 0 ]; then
    echo "错误：超边文件为空" >&2
    exit 1
fi

SPLIT=$(( TOTAL * 70 / 100 ))
if [ "$SPLIT" -lt 1 ]; then
    SPLIT=1
fi

REST=$(( TOTAL - SPLIT ))
if [ "$REST" -le 0 ]; then
    echo "警告：剩余部分为 0 行，无法执行插入/修改/删除。仅生成初始加载命令。" >&2
fi

echo "总超边数: $TOTAL, 初始(70%): $SPLIT, 剩余: $REST"

# 生成初始 70% 超边文件
head -n "$SPLIT" "$ORIGINAL_EDGES" > "$INITIAL_EDGES"

# 生成初始化命令文件（仅加载70%初始图与模式）
{
    echo "load_hg $INITIAL_EDGES $NODE_LABELS"
    echo "load_pattern $PATTERN_EDGES $PATTERN_NODE_LABELS"
} > "$INITIAL_COMMANDS"

# 生成动态操作命令文件（仅包含 add/modify/delete + quit）
{
    if [ "$REST" -gt 0 ]; then
        echo "# ===== 插入新增超边 ====="
        # 统计非空新增行数，确保 eid 与实际 add 次数一致
        ADDED_COUNT=$(tail -n "$REST" "$ORIGINAL_EDGES" | grep -v '^[[:space:]]*$' | wc -l | tr -d ' ')
        tail -n "$REST" "$ORIGINAL_EDGES" | while IFS= read -r line; do
            if [[ -z "${line// /}" ]]; then continue; fi
            IFS=',' read -r -a arr <<< "$line"
            n=${#arr[@]}
            echo -n "add_edge $n"
            for v in "${arr[@]}"; do echo -n " $((v-1))"; done
            echo
        done

        echo "# ===== 修改刚插入的超边（按顺序） ====="
        eid=$SPLIT
        tail -n "$REST" "$ORIGINAL_EDGES" | while IFS= read -r line; do
            if [[ -z "${line// /}" ]]; then continue; fi
            IFS=',' read -r -a arr <<< "$line"
            n=${#arr[@]}
            if [ "$n" -ge 2 ]; then
                tmp="${arr[0]}"; arr[0]="${arr[1]}"; arr[1]="$tmp"
                echo -n "modify_edge $eid $n"
                for v in "${arr[@]}"; do echo -n " $((v-1))"; done
                echo
            else
                # 单顶点边：保持不变，仍然发出一次 modify（无实质变化）
                echo "modify_edge $eid 1 $((arr[0]-1))"
            fi
            eid=$((eid+1))
        done

        echo "# ===== 删除刚插入/修改过的超边 ====="
        if [ "$ADDED_COUNT" -gt 0 ]; then
            for ((i=0; i<ADDED_COUNT; i++)); do
                echo "delete_edge $((SPLIT + i))"
            done
        fi
    fi
    echo "quit"
} > "$DYNAMIC_OPS"

# 生成合并总命令文件（初始化 + 动态操作）
cat "$INITIAL_COMMANDS" "$DYNAMIC_OPS" > "$COMMANDS_ALL"

echo "生成完成:"
echo "  初始超边文件: $INITIAL_EDGES"
echo "  初始化命令:   $INITIAL_COMMANDS"
echo "  动态操作命令: $DYNAMIC_OPS"
echo "  合并命令文件: $COMMANDS_ALL"
echo "可运行: cd $OHM_DIR && ./build/bin/interactive_dynamic_system < $COMMANDS_ALL"


