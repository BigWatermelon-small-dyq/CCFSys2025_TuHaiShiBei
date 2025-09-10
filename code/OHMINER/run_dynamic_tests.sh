#!/bin/bash

# 动态超图测试脚本
# 根据testdata下的文件夹名称执行相应的操作

# 检查是否在正确的目录
if [ ! -f "test/interactive_dynamic_system.cpp" ]; then
    echo "错误: 找不到 interactive_dynamic_system 可执行文件"
    echo "请确保在OHMiner目录下运行此脚本，并且已经编译了项目"
    exit 1
fi

# 检查testdata目录是否存在
if [ ! -d "../testdata" ]; then
    echo "错误: 找不到 testdata 目录"
    echo "请确保testdata目录存在并包含测试文件"
    exit 1
fi

echo "=== 动态超图测试系统启动 ==="
echo "正在启动 interactive_dynamic_system..."

# 创建临时输入文件
TEMP_INPUT=$(mktemp)

# 首先添加初始加载命令
cat > "$TEMP_INPUT" << EOF
load_hg ../hypergraphdataset/contact-primary-school/hyperedges-contact-primary-school.txt /home/w1nner/learn/OHMiner/hypergraphdataset/contact-primary-school/node-labels-contact-primary-school.txt
load_pattern ../pattern_edges.txt ../pattern_node_labels.txt
EOF

# 获取原始超图文件作为基准
ORIGINAL_FILE="../hypergraphdataset/contact-primary-school/hyperedges-contact-primary-school.txt"

# 遍历testdata目录下的所有文件夹
for folder in ../testdata/*/; do
    if [ -d "$folder" ]; then
        folder_name=$(basename "$folder")
        echo "处理文件夹: $folder_name"
        
        # 检查是否有超边文件
        hyperedge_file="$folder/hyperedges-contact-primary-school.txt"
        if [ ! -f "$hyperedge_file" ]; then
            echo "警告: 在 $folder_name 中找不到超边文件，跳过"
            continue
        fi
        
        case "$folder_name" in
            "addedges")
                echo "执行添加超边操作..."
                # 比较文件，找出新增的超边
                comm -13 <(sort "$ORIGINAL_FILE") <(sort "$hyperedge_file") | while read -r line; do
                    if [[ -n "$line" && ! "$line" =~ ^[[:space:]]*$ ]]; then
                        # 统计顶点个数并转换为空格分隔格式
                        vertex_count=$(echo "$line" | tr ',' '\n' | wc -l)
                        vertices_space=$(echo "$line" | tr ',' ' ')
                        echo "add_edge $vertex_count $vertices_space" >> "$TEMP_INPUT"
                        echo "添加超边: $vertex_count $vertices_space"
                    fi
                done
                ;;
            "deleteedges")
                echo "执行删除超边操作..."
                # 比较文件，找出被删除的超边
                comm -23 <(sort "$ORIGINAL_FILE") <(sort "$hyperedge_file") | while read -r line; do
                    if [[ -n "$line" && ! "$line" =~ ^[[:space:]]*$ ]]; then
                        # 需要找到超边的ID，这里我们使用行号作为ID
                        line_number=$(grep -n "^$line$" "$ORIGINAL_FILE" | cut -d: -f1)
                        if [[ -n "$line_number" ]]; then
                            echo "delete_edge $((line_number-1))" >> "$TEMP_INPUT"
                            echo "删除超边 $((line_number-1)): $line"
                        fi
                    fi
                done
                ;;
            "modifyedges")
                echo "执行修改超边操作..."
                # 对于修改操作，我们需要找出变化的超边
                # 这里我们比较两个文件，找出不同的行
                comm -3 <(sort "$ORIGINAL_FILE") <(sort "$hyperedge_file") | while read -r line; do
                    if [[ -n "$line" && ! "$line" =~ ^[[:space:]]*$ ]]; then
                        # 检查这行是否在原始文件中
                        if grep -q "^$line$" "$ORIGINAL_FILE"; then
                            # 这行在原始文件中，但在修改后的文件中可能被修改了
                            # 我们需要找到对应的修改后的行
                            line_number=$(grep -n "^$line$" "$ORIGINAL_FILE" | cut -d: -f1)
                            # 查找修改后的对应行
                            modified_line=$(sed -n "${line_number}p" "$hyperedge_file")
                            if [[ "$modified_line" != "$line" ]]; then
                                # 统计修改后超边的顶点个数并转换为空格分隔格式
                                vertex_count=$(echo "$modified_line" | tr ',' '\n' | wc -l)
                                vertices_space=$(echo "$modified_line" | tr ',' ' ')
                                echo "modify_edge $((line_number-1)) $vertex_count $vertices_space" >> "$TEMP_INPUT"
                                echo "修改超边 $((line_number-1)): $line -> $vertex_count $vertices_space"
                            fi
                        else
                            # 这行在修改后的文件中，是新增的
                            vertex_count=$(echo "$line" | tr ',' '\n' | wc -l)
                            vertices_space=$(echo "$line" | tr ',' ' ')
                            echo "add_edge $vertex_count $vertices_space" >> "$TEMP_INPUT"
                            echo "添加超边: $vertex_count $vertices_space"
                        fi
                    fi
                done
                ;;
            "插入节点和超边"|"删除节点和超边"|"修改节点")
                echo "处理节点操作文件夹: $folder_name"
                # 对于节点操作，我们需要比较节点标签文件
                original_node_file="../hypergraphdataset/contact-primary-school/node-labels-contact-primary-school.txt"
                modified_node_file="$folder/node-labels-contact-primary-school.txt"
                
                if [ -f "$modified_node_file" ]; then
                    echo "比较节点标签文件..."
                    # 比较节点标签文件，找出变化的节点
                    comm -3 <(sort "$original_node_file") <(sort "$modified_node_file") | while read -r line; do
                        if [[ -n "$line" && ! "$line" =~ ^[[:space:]]*$ ]]; then
                            # 解析节点ID和标签
                            node_id=$(echo "$line" | cut -d' ' -f1)
                            node_label=$(echo "$line" | cut -d' ' -f2)
                            
                            case "$folder_name" in
                                "插入节点和超边")
                                    echo "add_vertex $node_label" >> "$TEMP_INPUT"
                                    echo "添加节点: $node_id (标签: $node_label)"
                                    ;;
                                "删除节点和超边")
                                    echo "delete_vertex $node_id" >> "$TEMP_INPUT"
                                    echo "删除节点: $node_id"
                                    ;;
                                "修改节点")
                                    echo "set_vertex_label $node_id $node_label" >> "$TEMP_INPUT"
                                    echo "修改节点: $node_id -> 标签: $node_label"
                                    ;;
                            esac
                        fi
                    done
                fi
                ;;
            *)
                echo "未知的文件夹类型: $folder_name，跳过"
                ;;
        esac
        
        echo "完成处理文件夹: $folder_name"
        echo "" >> "$TEMP_INPUT"
    fi
done

# 添加退出命令
echo "quit" >> "$TEMP_INPUT"

echo "生成的命令序列:"
echo "=================="
cat "$TEMP_INPUT"
echo "=================="

echo ""
echo "正在启动系统并执行命令..."

# 启动interactive_dynamic_system并输入命令
./build/bin/interactive_dynamic_system < "$TEMP_INPUT"
./log.txt < "$TEMP_INPUT"
# 清理临时文件
rm "$TEMP_INPUT"

echo "测试完成!"
