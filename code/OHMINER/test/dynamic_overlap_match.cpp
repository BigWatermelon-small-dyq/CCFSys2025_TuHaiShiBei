#include "hypergraph.h"
#include "dataloader.h"
#include <iostream>
#include <vector>
#include <algorithm>

// 用法：
// ./dynamic_overlap_match <contact-primary-school/hyperedge.txt> <contact-primary-school/node.txt> <pattern_edges.txt> <pattern_node_labels.txt>
// 示例：
// ./dynamic_overlap_match ../hypergraphdataset/contact-primary-school/hypergraph.txt ../hypergraphdataset/contact-primary-school/node.txt ./pattern/ps_edges.txt ./pattern/ps_node_labels.txt
// ./dynamic_overlap_match /home/w1nner/learn/OHMiner/hypergraphdataset/contact-primary-school/hyperedges-contact-primary-school.txt /home/w1nner/learn/OHMiner/hypergraphdataset/contact-primary-school/node-labels-contact-primary-school.txt /home/w1nner/learn/OHMiner/pattern_edges.txt /home/w1nner/learn/OHMiner/pattern_node_labels.txt

static void print_edge(const std::vector<int>& e) {
    std::cout << "{";
    for (size_t i = 0; i < e.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << e[i];
    }
    std::cout << "}";
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <hypergraph_path> <node_label_path> <pattern_path> <pattern_node_label_path>" << std::endl;
        return 1;
    }
    std::cout << "Loading data..." << std::endl;
    const char* hypergraph_path = argv[1];
    const char* node_lable = argv[2];
    const char* pattern_path = argv[3];
    const char* pattern_node_lable = argv[4];

    std::cout << "Hypergraph: " << hypergraph_path << ", Node labels: " << node_lable << std::endl;
    hypergraph* hg = new hypergraph(0, 0);
    std::cout << "Hypergraph before loading: edges = " << hg->e_cnt << ", vertices = " << hg->v_cnt << std::endl;
    Dataloader dataloader;
    dataloader.load_DataGraph(hg, hypergraph_path, node_lable);
    std::cout << "Hypergraph loaded: edges = " << hg->e_cnt << ", vertices = " << hg->v_cnt << std::endl;
    
    std::cout << "Pattern: " << pattern_path << ", Pattern node labels: " << pattern_node_lable << std::endl;
    Pattern* pattern = new Pattern();
    dataloader.load_PatternGraph(pattern, pattern_path, pattern_node_lable);

    std::cout << "Preprocessing..." << std::endl;
    pattern->ComputeMatchingOrder(hg);
    pattern->generate_overlap_graph_sub();
    pattern->generate_execution_plan();

    std::cout << "Initial matching..." << std::endl;
    auto t1 = get_wall_time();
    hg->overlap_aware_pattern_matching(pattern);
    auto t2 = get_wall_time();
    std::cout << "time: " << (t2 - t1) << "s" << std::endl;

    // ===== 动态操作：增删改边与点 =====
    std::cout << "\nDynamic operations begin..." << std::endl;

    // 1) 插入超边：选择前三个顶点构造一条新边
    std::vector<int> e_add;
    for (int v = 0; v < std::min(3, hg->v_cnt); ++v) e_add.push_back(v);
    int new_eid = -1;
    if (!e_add.empty()) {
        new_eid = hg->add_hyperedge(e_add);
        std::cout << "Added edge e" << new_eid << " = "; print_edge(hg->hyperedge2node[new_eid]); std::cout << std::endl;
    }

    // 2) 修改一条存在的超边（若存在），将其更换为另一组顶点
    if (hg->e_cnt > 1) {
        int target_e = 0;
        std::vector<int> new_vertices;
        for (int v = std::max(0, hg->v_cnt - 3); v < hg->v_cnt; ++v) new_vertices.push_back(v);
        if (!new_vertices.empty()) {
            hg->modify_hyperedge_vertices(target_e, new_vertices);
            std::cout << "Modified edge e" << target_e << " -> "; print_edge(hg->hyperedge2node[target_e]); std::cout << std::endl;
        }
    }

    // 3) 删除一条存在的超边（若存在新增边则删新增的，否则删最后一条）
    if (hg->e_cnt > 0) {
        int del_e = (new_eid >= 0 ? new_eid : (hg->e_cnt - 1));
        std::cout << "Deleting edge e" << del_e << std::endl;
        hg->delete_hyperedge(del_e);
        if (del_e < (int)hg->hyperedge2node.size()) {
            if (hg->hyperedge2node[del_e].empty()) std::cout << "Edge e" << del_e << " deleted." << std::endl;
        }
    }

    // 4) 修改一个顶点标签（若存在顶点）并对相关边生效
    if (hg->v_cnt > 0) {
        int v = 0;
        int new_label = (hg->v_to_lable[v] + 1000);
        hg->set_vertex_label(v, new_label);
        std::cout << "Set vertex v" << v << " label -> " << new_label << std::endl;
    }

    // 5) 顶点级别的增删：若存在足够边，向第一条边添加一个最大ID顶点，随后删除该顶点
    if (hg->e_cnt > 0) {
        int tgt_e = 0;
        int added_v = hg->add_vertex(777); // 新标签
        hg->add_vertex_to_hyperedge(tgt_e, added_v);
        std::cout << "Added vertex v" << added_v << " to e" << tgt_e << std::endl;
        hg->remove_vertex_from_hyperedge(tgt_e, added_v);
        std::cout << "Removed vertex v" << added_v << " from e" << tgt_e << std::endl;
        hg->delete_vertex(added_v);
        std::cout << "Deleted vertex v" << added_v << std::endl;
    }

    // 动态操作后再次匹配
    std::cout << "\nMatching after dynamic updates..." << std::endl;
    auto t3 = get_wall_time();
    hg->overlap_aware_pattern_matching(pattern);
    auto t4 = get_wall_time();
    std::cout << "time: " << (t4 - t3) << "s" << std::endl;

    delete pattern;
    delete hg;
    return 0;
}


