#include "hypergraph.h"
#include "dataloader.h"
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <queue>

// 全局对象
static hypergraph* g_hg = nullptr;
static Pattern* g_pattern = nullptr;
static long long g_match_count = 0;

// 从一组超边构造子图（顶点重标号为紧凑的0..k-1；保留原标签）
static hypergraph* build_subgraph_from_edges(const hypergraph* base, const std::vector<int>& edges) {
    if (!base || edges.empty()) return nullptr;
    std::unordered_set<int> vertex_set;
    for (int e : edges) {
        if (e < 0 || e >= (int)base->hyperedge2node.size()) continue;
        for (int v : base->hyperedge2node[e]) vertex_set.insert(v);
    }
    // 旧 -> 新 顶点映射
    std::vector<int> old2new(base->v_cnt, -1);
    std::vector<int> new2old;
    new2old.reserve(vertex_set.size());
    int nv = 0;
    for (int v : vertex_set) {
        old2new[v] = nv++;
        new2old.push_back(v);
    }

    // 创建子超图
    hypergraph* sub = new hypergraph(0, (int)vertex_set.size());
    sub->v_to_lable.resize(vertex_set.size());
    for (int new_v = 0; new_v < (int)new2old.size(); ++new_v) {
        int old_v = new2old[new_v];
        sub->v_to_lable[new_v] = base->v_to_lable[old_v];
        sub->node2hyperedge.push_back(std::vector<int>());
    }

    // 通过新增接口逐条插入边，保持所有元数据一致
    for (int e : edges) {
        if (e < 0 || e >= (int)base->hyperedge2node.size()) continue;
        std::vector<int> he_new;
        he_new.reserve(base->hyperedge2node[e].size());
        for (int old_v : base->hyperedge2node[e]) he_new.push_back(old2new[old_v]);
        sub->add_hyperedge(he_new);
    }
    return sub;
}

// 在当前全局超图上，对受影响的边集合构成子图并匹配，返回子图匹配数量
static long long match_on_edge_subgraph(const std::vector<int>& affected_edges) {
    if (!g_hg || !g_pattern || affected_edges.empty()) return 0;
    hypergraph* sub = build_subgraph_from_edges(g_hg, affected_edges);
    if (!sub) return 0;
    Pattern p = *g_pattern; // 拷贝模式，以免改动全局
    p.ComputeMatchingOrder(sub);
    p.generate_overlap_graph_sub();
    p.generate_execution_plan();
    long long cnt = sub->overlap_aware_pattern_matching(&p);
    delete sub;
    return cnt;
}

// 计算受影响的边集合：由一个顶点集合触发
static std::vector<int> collect_edges_by_vertices(const std::vector<int>& vertices) {
    std::unordered_set<int> edges;
    if (!g_hg) return {};
    for (int v : vertices) {
        if (v < 0 || v >= (int)g_hg->node2hyperedge.size()) continue;
        for (int e : g_hg->node2hyperedge[v]) edges.insert(e);
    }
    // 加入邻接边
    std::unordered_set<int> more;
    for (int e : edges) {
        if (e < 0 || e >= (int)g_hg->hyperedge_adj.size()) continue;
        for (int nb : g_hg->hyperedge_adj[e]) more.insert(nb);
    }
    for (int e : more) edges.insert(e);
    return std::vector<int>(edges.begin(), edges.end());
}

// 计算受影响的边集合：由一个边集合触发（加入其邻居）
static std::vector<int> collect_edges_with_neighbors(const std::vector<int>& base_edges) {
    std::unordered_set<int> edges(base_edges.begin(), base_edges.end());
    if (!g_hg) return {};
    std::unordered_set<int> more;
    for (int e : base_edges) {
        if (e < 0 || e >= (int)g_hg->hyperedge_adj.size()) continue;
        for (int nb : g_hg->hyperedge_adj[e]) more.insert(nb);
    }
    for (int e : more) edges.insert(e);
    return std::vector<int>(edges.begin(), edges.end());
}

// 计算受影响的边集合：考虑边修改可能影响的更广泛范围
static std::vector<int> collect_edges_for_modification(int modified_edge_id, const std::vector<int>& old_vertices, const std::vector<int>& new_vertices) {
    if (!g_hg) return {};
    
    std::unordered_set<int> affected_edges;
    
    // 1. 包含被修改的边本身
    affected_edges.insert(modified_edge_id);
    
    // 2. 包含所有旧顶点的关联边
    for (int v : old_vertices) {
        if (v >= 0 && v < (int)g_hg->node2hyperedge.size()) {
            for (int e : g_hg->node2hyperedge[v]) {
                affected_edges.insert(e);
            }
        }
    }
    
    // 3. 包含所有新顶点的关联边
    for (int v : new_vertices) {
        if (v >= 0 && v < (int)g_hg->node2hyperedge.size()) {
            for (int e : g_hg->node2hyperedge[v]) {
                affected_edges.insert(e);
            }
        }
    }
    
    // 4. 包含所有受影响边的邻居边
    std::unordered_set<int> more;
    for (int e : affected_edges) {
        if (e >= 0 && e < (int)g_hg->hyperedge_adj.size()) {
            for (int nb : g_hg->hyperedge_adj[e]) {
                more.insert(nb);
            }
        }
    }
    for (int e : more) affected_edges.insert(e);
    
    return std::vector<int>(affected_edges.begin(), affected_edges.end());
}

// 对当前全图强制匹配并更新全局计数
static void recompute_global_match() {
    if (!g_hg || !g_pattern) return;
    Pattern p = *g_pattern;
    p.ComputeMatchingOrder(g_hg);
    p.generate_overlap_graph_sub();
    p.generate_execution_plan();
    g_match_count = g_hg->overlap_aware_pattern_matching(&p);
    std::cout << "[global] pattern matches = " << g_match_count << std::endl;
}

static void try_initial_match_after_load() {
    if (g_hg && g_pattern) {
        std::cout << "[info] both hypergraph and pattern loaded. Running initial match..." << std::endl;
        recompute_global_match();
    }
}

static void print_help() {
    std::cout << "Commands:\n"
              << "  load_hg <hyperedges_file> <node_labels_file>\n"
              << "  load_pattern <pattern_edges_file> <pattern_node_labels_file>\n"
              << "  add_vertex <label>\n"
              << "  delete_vertex <vid>\n"
              << "  set_vertex_label <vid> <new_label>\n"
              << "  add_edge <n> v1 v2 ... vn\n"
              << "  delete_edge <eid>\n"
              << "  modify_edge <eid> <n> v1 v2 ... vn\n"
              << "  modify_edge_global <eid> <n> v1 v2 ... vn  (force global recomputation)\n"
              << "  match\n"
              << "  help\n"
              << "  quit" << std::endl;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    print_help();
    Dataloader loader;

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string cmd; iss >> cmd;

        if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "load_hg") {
            std::string efile, vfile; iss >> efile >> vfile;
            if (efile.empty() || vfile.empty()) { std::cout << "usage: load_hg <hyperedges> <node_labels>" << std::endl; continue; }
            if (g_hg) { delete g_hg; g_hg = nullptr; }
            g_hg = new hypergraph(0, 0);
            loader.load_DataGraph(g_hg, efile.c_str(), vfile.c_str());
            std::cout << "[load_hg] edges=" << g_hg->e_cnt << ", vertices=" << g_hg->v_cnt << std::endl;
            try_initial_match_after_load();
        } else if (cmd == "load_pattern") {
            std::string efile, vfile; iss >> efile >> vfile;
            if (efile.empty() || vfile.empty()) { std::cout << "usage: load_pattern <pattern_edges> <pattern_node_labels>" << std::endl; continue; }
            if (g_pattern) { delete g_pattern; g_pattern = nullptr; }
            g_pattern = new Pattern();
            loader.load_PatternGraph(g_pattern, efile.c_str(), vfile.c_str());
            std::cout << "[load_pattern] e_cnt=" << g_pattern->e_cnt << ", v_cnt=" << g_pattern->v_cnt << std::endl;
            try_initial_match_after_load();
        } else if (cmd == "add_vertex") {
            int label; if (!(iss >> label)) { std::cout << "usage: add_vertex <label>" << std::endl; continue; }
            if (!g_hg) { std::cout << "[error] load hypergraph first" << std::endl; continue; }
            int vid = g_hg->add_vertex(label);
            std::cout << "[add_vertex] v" << vid << " label=" << label << std::endl;
            if (g_pattern) {
                auto affected = collect_edges_by_vertices({vid});
                long long sub = match_on_edge_subgraph(affected);
                g_match_count += sub; // 新增：增加匹配数
                std::cout << "[subgraph match] affected edges=" << affected.size() << ", +" << sub << std::endl;
                std::cout << "[total] pattern matches = " << g_match_count << std::endl;
            }
        } else if (cmd == "delete_vertex") {
            int vid; if (!(iss >> vid)) { std::cout << "usage: delete_vertex <vid>" << std::endl; continue; }
            if (!g_hg) { std::cout << "[error] load hypergraph first" << std::endl; continue; }
            auto affected = collect_edges_by_vertices({vid});
            long long sub_before = 0;
            if (g_pattern) sub_before = match_on_edge_subgraph(affected);
            g_hg->delete_vertex(vid);
            std::cout << "[delete_vertex] v" << vid << std::endl;
            if (g_pattern) {
                // 删除：减少匹配数（使用删除前子图计数）
                g_match_count -= sub_before;
                std::cout << "[subgraph match] affected edges=" << affected.size() << ", -" << sub_before << std::endl;
                std::cout << "[total] pattern matches = " << g_match_count << std::endl;
            }
        } else if (cmd == "set_vertex_label") {
            int vid, nl; if (!(iss >> vid >> nl)) { std::cout << "usage: set_vertex_label <vid> <new_label>" << std::endl; continue; }
            if (!g_hg) { std::cout << "[error] load hypergraph first" << std::endl; continue; }
            auto affected = collect_edges_by_vertices({vid});
            long long sub_before = 0;
            if (g_pattern) sub_before = match_on_edge_subgraph(affected);
            g_hg->set_vertex_label(vid, nl);
            std::cout << "[set_vertex_label] v" << vid << " -> label=" << nl << std::endl;
            if (g_pattern) {
                long long sub_after = match_on_edge_subgraph(affected);
                long long delta = sub_after - sub_before;
                g_match_count += delta; // 顶点修改：按差值更新
                std::cout << "[subgraph match] affected edges=" << affected.size() << ", delta=" << delta << std::endl;
                std::cout << "[total] pattern matches = " << g_match_count << std::endl;
            }
        } else if (cmd == "add_edge") {
            int n; if (!(iss >> n) || n <= 0) { std::cout << "usage: add_edge <n> v1 v2 ... vn" << std::endl; continue; }
            if (!g_hg) { std::cout << "[error] load hypergraph first" << std::endl; continue; }
            std::vector<int> vs; vs.reserve(n);
            for (int i = 0; i < n; ++i) { int v; if (!(iss >> v)) { std::cout << "[error] need " << n << " vertices" << std::endl; vs.clear(); break; } vs.push_back(v); }
            if (vs.empty()) continue;
            int eid = g_hg->add_hyperedge(vs);
            std::cout << "[add_edge] e" << eid << " size=" << vs.size() << std::endl;
            if (g_pattern) {
                auto affected = collect_edges_with_neighbors({eid});
                long long sub = match_on_edge_subgraph(affected);
                g_match_count += sub; // 新增边：增加匹配数
                std::cout << "[subgraph match] affected edges=" << affected.size() << ", +" << sub << std::endl;
                std::cout << "[total] pattern matches = " << g_match_count << std::endl;
            }
        } else if (cmd == "delete_edge") {
            int eid; if (!(iss >> eid)) { std::cout << "usage: delete_edge <eid>" << std::endl; continue; }
            if (!g_hg) { std::cout << "[error] load hypergraph first" << std::endl; continue; }
            auto affected = collect_edges_with_neighbors({eid});
            long long sub_before = 0;
            if (g_pattern) sub_before = match_on_edge_subgraph(affected);
            g_hg->delete_hyperedge(eid);
            std::cout << "[delete_edge] e" << eid << std::endl;
            if (g_pattern) {
                // 删除边：减少匹配数（使用删除前子图计数）
                g_match_count -= sub_before;
                std::cout << "[subgraph match] affected edges=" << affected.size() << ", -" << sub_before << std::endl;
                std::cout << "[total] pattern matches = " << g_match_count << std::endl;
            }
        } else if (cmd == "modify_edge") {
            int eid, n; if (!(iss >> eid >> n) || n < 0) { std::cout << "usage: modify_edge <eid> <n> v1 v2 ... vn" << std::endl; continue; }
            if (!g_hg) { std::cout << "[error] load hypergraph first" << std::endl; continue; }
            std::vector<int> vs; vs.reserve(n);
            for (int i = 0; i < n; ++i) { int v; if (!(iss >> v)) { std::cout << "[error] need " << n << " vertices" << std::endl; vs.clear(); break; } vs.push_back(v); }
            if (vs.empty() && n > 0) continue;
            
            // 获取修改前的顶点信息
            std::vector<int> old_vertices;
            if (eid >= 0 && eid < (int)g_hg->hyperedge2node.size()) {
                old_vertices = g_hg->hyperedge2node[eid];
            }
            
            // 修改前收集受影响的边（基于旧顶点信息）
            auto affected_before = collect_edges_for_modification(eid, old_vertices, vs);
            long long sub_before = 0;
            if (g_pattern && !affected_before.empty()) {
                sub_before = match_on_edge_subgraph(affected_before);
            }
            
            // 执行边修改
            g_hg->modify_hyperedge_vertices(eid, vs);
            std::cout << "[modify_edge] e" << eid << " new_size=" << vs.size() << std::endl;
            
            if (g_pattern) {
                // 修改后重新收集受影响的边（基于新顶点信息）
                auto affected_after = collect_edges_for_modification(eid, old_vertices, vs);
                long long sub_after = 0;
                if (!affected_after.empty()) {
                    sub_after = match_on_edge_subgraph(affected_after);
                }
                
                // 计算增量：修改后的匹配数 - 修改前的匹配数
                long long delta = sub_after - sub_before;
                g_match_count += delta;
                
                // 如果子图太小或delta为0，考虑使用全局重新计算
                // if (affected_after.size() < 10 || delta == 0) {
                //     std::cout << "[warning] Small subgraph or zero delta detected. Consider global recomputation." << std::endl;
                // }
                
                std::cout << "[subgraph match] affected edges before=" << affected_before.size() 
                         << ", after=" << affected_after.size() 
                         << ", delta=" << delta << std::endl;
                std::cout << "[total] pattern matches = " << g_match_count << std::endl;
            }
        } else if (cmd == "modify_edge_global") {
            int eid, n; if (!(iss >> eid >> n) || n < 0) { std::cout << "usage: modify_edge_global <eid> <n> v1 v2 ... vn" << std::endl; continue; }
            if (!g_hg) { std::cout << "[error] load hypergraph first" << std::endl; continue; }
            std::vector<int> vs; vs.reserve(n);
            for (int i = 0; i < n; ++i) { int v; if (!(iss >> v)) { std::cout << "[error] need " << n << " vertices" << std::endl; vs.clear(); break; } vs.push_back(v); }
            if (vs.empty() && n > 0) continue;
            
            // 直接执行边修改，然后全局重新计算
            g_hg->modify_hyperedge_vertices(eid, vs);
            std::cout << "[modify_edge_global] e" << eid << " new_size=" << vs.size() << std::endl;
            
            if (g_pattern) {
                // 强制全局重新计算
                std::cout << "[info] Performing global recomputation..." << std::endl;
                recompute_global_match();
            }
        } else if (cmd == "match") {
            if (!g_hg || !g_pattern) { std::cout << "[error] load hypergraph and pattern first" << std::endl; continue; }
            recompute_global_match();
        } else {
            std::cout << "[error] unknown command. type 'help'." << std::endl;
        }
    }

    if (g_pattern) delete g_pattern;
    if (g_hg) delete g_hg;
    return 0;
}


