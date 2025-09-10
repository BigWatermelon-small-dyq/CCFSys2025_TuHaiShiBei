#include "hypergraph.h"
#include "dataloader.h"
#include <iostream>
#include <cassert>
#include <algorithm>
#include <set>

class DynamicOperationsTest {
private:
    hypergraph* hg;
    
    // 验证数据结构一致性的辅助函数
    bool verify_consistency() {
        // 验证 node2hyperedge 与 hyperedge2node 的一致性
        for (int v = 0; v < (int)hg->node2hyperedge.size(); ++v) {
            for (int eid : hg->node2hyperedge[v]) {
                if (eid >= (int)hg->hyperedge2node.size()) return false;
                auto& vertices = hg->hyperedge2node[eid];
                if (std::find(vertices.begin(), vertices.end(), v) == vertices.end()) {
                    std::cout << "Inconsistency: vertex " << v << " claims edge " << eid 
                              << " but edge doesn't contain vertex" << std::endl;
                    return false;
                }
            }
        }
        
        for (int eid = 0; eid < (int)hg->hyperedge2node.size(); ++eid) {
            for (int v : hg->hyperedge2node[eid]) {
                if (v >= (int)hg->node2hyperedge.size()) return false;
                auto& edges = hg->node2hyperedge[v];
                if (std::find(edges.begin(), edges.end(), eid) == edges.end()) {
                    std::cout << "Inconsistency: edge " << eid << " contains vertex " << v 
                              << " but vertex doesn't reference edge" << std::endl;
                    return false;
                }
            }
        }
        
        // 验证 hedge_to_kid 与 HyperGraph_table 的一致性
        for (int eid = 0; eid < (int)hg->hedge_to_kid.size(); ++eid) {
            int kid = hg->hedge_to_kid[eid];
            if (kid >= 0) {
                auto it = hg->HyperGraph_table.find(kid);
                if (it == hg->HyperGraph_table.end()) return false;
                auto& stable = it->second;
                if (stable.sedge.find(eid) == stable.sedge.end()) {
                    std::cout << "Inconsistency: edge " << eid << " has kid " << kid 
                              << " but not in HyperGraph_table" << std::endl;
                    return false;
                }
            }
        }
        
        return true;
    }
    
    void print_hypergraph_state() {
        std::cout << "\n=== Hypergraph State ===" << std::endl;
        std::cout << "Vertices: " << hg->v_cnt << ", Edges: " << hg->e_cnt << std::endl;
        
        std::cout << "Vertex labels: ";
        for (int i = 0; i < (int)hg->v_to_lable.size(); ++i) {
            std::cout << "v" << i << ":" << hg->v_to_lable[i] << " ";
        }
        std::cout << std::endl;
        
        std::cout << "Hyperedges:" << std::endl;
        for (int eid = 0; eid < (int)hg->hyperedge2node.size(); ++eid) {
            auto& vertices = hg->hyperedge2node[eid];
            if (!vertices.empty()) {
                std::cout << "  e" << eid << ": {";
                for (int i = 0; i < (int)vertices.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << vertices[i];
                }
                std::cout << "}" << std::endl;
            }
        }
        std::cout << "========================\n" << std::endl;
    }

public:
    DynamicOperationsTest() {
        hg = new hypergraph(0, 0);
    }
    
    ~DynamicOperationsTest() {
        delete hg;
    }
    
    void test_add_vertex() {
        std::cout << "Testing add_vertex..." << std::endl;
        
        // 添加几个顶点
        int v0 = hg->add_vertex(1);  // 标签为1
        int v1 = hg->add_vertex(2);  // 标签为2
        int v2 = hg->add_vertex(1);  // 标签为1
        
        assert(v0 == 0 && v1 == 1 && v2 == 2);
        assert(hg->v_cnt == 3);
        assert(hg->v_to_lable[0] == 1);
        assert(hg->v_to_lable[1] == 2);
        assert(hg->v_to_lable[2] == 1);
        
        std::cout << "✓ add_vertex test passed" << std::endl;
    }
    
    void test_add_hyperedge() {
        std::cout << "Testing add_hyperedge..." << std::endl;
        
        // 确保有足够的顶点
        while (hg->v_cnt < 5) {
            hg->add_vertex(hg->v_cnt + 1);
        }
        
        // 添加超边 {0, 1, 2}
        std::vector<int> vertices1 = {0, 1, 2};
        int e0 = hg->add_hyperedge(vertices1);
        
        // 添加超边 {1, 2, 3}
        std::vector<int> vertices2 = {1, 2, 3};
        int e1 = hg->add_hyperedge(vertices2);
        
        // 添加超边 {2, 3, 4}
        std::vector<int> vertices3 = {2, 3, 4};
        int e2 = hg->add_hyperedge(vertices3);
        
        assert(e0 == 0 && e1 == 1 && e2 == 2);
        assert(hg->e_cnt == 3);
        
        // 验证超边内容
        assert(hg->hyperedge2node[0] == std::vector<int>({0, 1, 2}));
        assert(hg->hyperedge2node[1] == std::vector<int>({1, 2, 3}));
        assert(hg->hyperedge2node[2] == std::vector<int>({2, 3, 4}));
        
        // 验证邻接关系
        assert(std::find(hg->hyperedge_adj[0].begin(), hg->hyperedge_adj[0].end(), 1) != hg->hyperedge_adj[0].end());
        assert(std::find(hg->hyperedge_adj[1].begin(), hg->hyperedge_adj[1].end(), 0) != hg->hyperedge_adj[1].end());
        assert(std::find(hg->hyperedge_adj[1].begin(), hg->hyperedge_adj[1].end(), 2) != hg->hyperedge_adj[1].end());
        
        assert(verify_consistency());
        std::cout << "✓ add_hyperedge test passed" << std::endl;
    }
    
    void test_modify_hyperedge() {
        std::cout << "Testing modify_hyperedge_vertices..." << std::endl;
        
        print_hypergraph_state();
        
        // 修改超边0：从 {0, 1, 2} 改为 {0, 3, 4}
        std::vector<int> new_vertices = {0, 3, 4};
        hg->modify_hyperedge_vertices(0, new_vertices);
        
        print_hypergraph_state();
        
        // 验证修改结果
        assert(hg->hyperedge2node[0] == std::vector<int>({0, 3, 4}));
        
        // 验证 node2hyperedge 更新正确
        auto& node1_edges = hg->node2hyperedge[1];
        assert(std::find(node1_edges.begin(), node1_edges.end(), 0) == node1_edges.end());
        
        auto& node3_edges = hg->node2hyperedge[3];
        assert(std::find(node3_edges.begin(), node3_edges.end(), 0) != node3_edges.end());
        
        assert(verify_consistency());
        std::cout << "✓ modify_hyperedge_vertices test passed" << std::endl;
    }
    
    void test_add_remove_vertex_to_hyperedge() {
        std::cout << "Testing add/remove vertex to/from hyperedge..." << std::endl;
        
        // 向超边1添加顶点0
        hg->add_vertex_to_hyperedge(1, 0);
        print_hypergraph_state();
        
        // 验证顶点0已添加到超边1
        auto& edge1_vertices = hg->hyperedge2node[1];
        assert(std::find(edge1_vertices.begin(), edge1_vertices.end(), 0) != edge1_vertices.end());
        
        // 从超边1移除顶点2
        hg->remove_vertex_from_hyperedge(1, 2);
        print_hypergraph_state();
        
        // 验证顶点2已从超边1移除
        auto& edge1_vertices_after = hg->hyperedge2node[1];
        assert(std::find(edge1_vertices_after.begin(), edge1_vertices_after.end(), 2) == edge1_vertices_after.end());
        
        assert(verify_consistency());
        std::cout << "✓ add/remove vertex to/from hyperedge test passed" << std::endl;
    }
    
    void test_delete_hyperedge() {
        std::cout << "Testing delete_hyperedge..." << std::endl;
        
        print_hypergraph_state();
        
        // 删除超边2
        hg->delete_hyperedge(2);
        
        print_hypergraph_state();
        
        // 验证超边2已被删除（顶点集为空）
        assert(hg->hyperedge2node[2].empty());
        
        // 验证相关顶点的 incident 列表已更新
        for (int v = 0; v < hg->v_cnt; ++v) {
            auto& edges = hg->node2hyperedge[v];
            assert(std::find(edges.begin(), edges.end(), 2) == edges.end());
        }
        
        assert(verify_consistency());
        std::cout << "✓ delete_hyperedge test passed" << std::endl;
    }
    
    void test_set_vertex_label() {
        std::cout << "Testing set_vertex_label..." << std::endl;
        
        // 修改顶点3的标签从原来的4改为10
        hg->set_vertex_label(3, 10);
        
        assert(hg->v_to_lable[3] == 10);
        
        assert(verify_consistency());
        std::cout << "✓ set_vertex_label test passed" << std::endl;
    }
    
    void test_delete_vertex() {
        std::cout << "Testing delete_vertex..." << std::endl;
        
        print_hypergraph_state();
        
        // 删除顶点3
        hg->delete_vertex(3);
        
        print_hypergraph_state();
        
        // 验证顶点3的 incident 列表为空
        if (3 < (int)hg->node2hyperedge.size()) {
            assert(hg->node2hyperedge[3].empty());
        }
        
        // 验证包含顶点3的超边已更新
        for (int eid = 0; eid < (int)hg->hyperedge2node.size(); ++eid) {
            auto& vertices = hg->hyperedge2node[eid];
            assert(std::find(vertices.begin(), vertices.end(), 3) == vertices.end());
        }
        
        assert(verify_consistency());
        std::cout << "✓ delete_vertex test passed" << std::endl;
    }
    
    void test_edge_cases() {
        std::cout << "Testing edge cases..." << std::endl;
        
        // 测试添加空超边
        std::vector<int> empty_vertices;
        int result = hg->add_hyperedge(empty_vertices);
        assert(result == -1);
        
        // 测试添加重复顶点的超边
        std::vector<int> duplicate_vertices = {0, 1, 1, 0, 2};
        int eid = hg->add_hyperedge(duplicate_vertices);
        assert(eid >= 0);
        auto& clean_vertices = hg->hyperedge2node[eid];
        assert(clean_vertices == std::vector<int>({0, 1, 2}));
        
        // 测试删除不存在的超边
        hg->delete_hyperedge(999);  // 应该安全处理
        
        // 测试修改不存在的顶点标签
        hg->set_vertex_label(999, 100);  // 应该扩展数组
        if (999 < (int)hg->v_to_lable.size()) {
            assert(hg->v_to_lable[999] == 100);
        }
        
        assert(verify_consistency());
        std::cout << "✓ edge cases test passed" << std::endl;
    }
    
    void test_comprehensive_scenario() {
        std::cout << "Testing comprehensive scenario..." << std::endl;
        
        // 创建一个全新的超图进行综合测试
        delete hg;
        hg = new hypergraph(0, 0);
        
        // 场景：构建一个小型超图并进行各种操作
        
        // 1. 添加顶点
        for (int i = 0; i < 6; ++i) {
            hg->add_vertex(i % 3);  // 标签为0, 1, 2循环
        }
        
        // 2. 添加超边
        hg->add_hyperedge({0, 1, 2});     // e0
        hg->add_hyperedge({1, 2, 3});     // e1  
        hg->add_hyperedge({2, 3, 4});     // e2
        hg->add_hyperedge({3, 4, 5});     // e3
        
        std::cout << "Initial state:" << std::endl;
        print_hypergraph_state();
        
        // 3. 修改操作
        hg->modify_hyperedge_vertices(1, {0, 1, 4, 5});  // 修改e1
        hg->add_vertex_to_hyperedge(2, 0);                // 向e2添加顶点0
        hg->remove_vertex_from_hyperedge(3, 5);           // 从e3移除顶点5
        
        std::cout << "After modifications:" << std::endl;
        print_hypergraph_state();
        
        // 4. 删除操作
        hg->delete_hyperedge(0);         // 删除e0
        hg->delete_vertex(4);            // 删除顶点4
        
        std::cout << "After deletions:" << std::endl;
        print_hypergraph_state();
        
        // 5. 验证最终状态的一致性
        assert(verify_consistency());
        
        std::cout << "✓ comprehensive scenario test passed" << std::endl;
    }
    
    void run_all_tests() {
        std::cout << "Running Dynamic Operations Tests..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        test_add_vertex();
        test_add_hyperedge();
        test_modify_hyperedge();
        test_add_remove_vertex_to_hyperedge();
        test_delete_hyperedge();
        test_set_vertex_label();
        test_delete_vertex();
        test_edge_cases();
        test_comprehensive_scenario();
        
        std::cout << "========================================" << std::endl;
        std::cout << "All tests passed! ✓" << std::endl;
    }
};

int main() {
    try {
        DynamicOperationsTest test;
        test.run_all_tests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
