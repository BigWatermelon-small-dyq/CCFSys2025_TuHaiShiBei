#include "../include/dynamic_hypergraph.h"
#include "../include/dataloader.h"
#include <iostream>
#include <vector>
#include <chrono>

void test_dynamic_hypergraph_basic_operations() {
    std::cout << "=== 测试动态超图基本操作 ===" << std::endl;
    
    // 创建一个空的动态超图
    DynamicHypergraph dynamic_hg;
    
    // 插入一些超边
    std::cout << "插入超边..." << std::endl;
    dynamic_hg.insert_hyperedge(0, {0, 1, 2});
    dynamic_hg.insert_hyperedge(1, {1, 2, 3});
    dynamic_hg.insert_hyperedge(2, {2, 3, 4});
    dynamic_hg.insert_hyperedge(3, {0, 2, 4});
    
    // 显示系统状态
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 修改超边
    std::cout << "\n修改超边..." << std::endl;
    dynamic_hg.modify_hyperedge(1, {1, 2, 3, 5}); // 添加顶点5
    
    // 显示系统状态
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 删除超边
    std::cout << "\n删除超边..." << std::endl;
    dynamic_hg.delete_hyperedge(2);
    
    // 显示系统状态
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 插入顶点
    std::cout << "\n插入顶点..." << std::endl;
    dynamic_hg.insert_vertex(6, {0, 3}); // 顶点6与超边0和3相关
    
    // 显示系统状态
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 执行剪枝操作
    std::cout << "\n执行剪枝操作..." << std::endl;
    dynamic_hg.perform_pruning();
    
    // 显示最终系统状态
    std::cout << "\n最终系统状态:" << std::endl;
    std::cout << dynamic_hg.get_system_status() << std::endl;
}

void test_dynamic_hypergraph_from_static() {
    std::cout << "\n=== 测试从静态超图创建动态超图 ===" << std::endl;
    
    // 创建一个简单的静态超图
    hypergraph static_hg(4, 6); // 4个超边，6个顶点
    
    // 设置超边到顶点的映射
    static_hg.hyperedge2node = {
        {0, 1, 2},    // 超边0
        {1, 2, 3},    // 超边1
        {2, 3, 4},    // 超边2
        {0, 2, 4}     // 超边3
    };
    
    // 设置顶点到超边的映射
    static_hg.node2hyperedge = {
        {0, 3},       // 顶点0
        {0, 1},       // 顶点1
        {0, 1, 2, 3}, // 顶点2
        {1, 2},       // 顶点3
        {2, 3},       // 顶点4
        {}             // 顶点5
    };
    
    // 设置超边邻接关系
    static_hg.hyperedgeAdj = {
        {{1, {2}}, {2, {1}}, {3, {0}}},  // 超边0的邻接
        {{0, {1}}, {2, {2}}, {3, {1}}},  // 超边1的邻接
        {{0, {2}}, {1, {2}}, {3, {2}}},  // 超边2的邻接
        {{0, {0}}, {1, {1}}, {2, {2}}}   // 超边3的邻接
    };
    
    // 从静态超图创建动态超图
    DynamicHypergraph dynamic_hg(&static_hg);
    
    // 显示系统状态
    std::cout << "从静态超图创建后的状态:" << std::endl;
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 获取OIG信息
    const auto* oig = dynamic_hg.get_oig();
    if (oig) {
        auto execution_sequence = oig->generate_execution_sequence();
        auto execution_plan = oig->generate_execution_plan();
        
        std::cout << "\n执行序列长度: " << execution_sequence.size() << std::endl;
        std::cout << "执行计划层数: " << execution_plan.size() << std::endl;
    }
    
    // 获取度感知存储信息
    const auto* degree_storage = dynamic_hg.get_degree_storage();
    if (degree_storage) {
        auto degree_stats = degree_storage->get_degree_statistics();
        std::cout << "\n度值分布:" << std::endl;
        for (const auto& pair : degree_stats) {
            std::cout << "度值 " << pair.first << ": " << pair.second << " 个超边" << std::endl;
        }
    }
}

void test_dynamic_pattern_matching() {
    std::cout << "\n=== 测试动态模式匹配 ===" << std::endl;
    
    // 创建一个简单的静态超图
    hypergraph static_hg(4, 6);
    static_hg.hyperedge2node = {
        {0, 1, 2},
        {1, 2, 3},
        {2, 3, 4},
        {0, 2, 4}
    };
    
    // 创建动态超图
    DynamicHypergraph dynamic_hg(&static_hg);
    
    // 创建模式（这里简化处理）
    Pattern pattern;
    pattern.e_cnt = 2;
    pattern.v_cnt = 3;
    pattern.hyperedge2node = {
        {0, 1},
        {1, 2}
    };
    
    // 执行动态模式匹配
    std::cout << "执行动态模式匹配..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    int64_t match_count = dynamic_hg.dynamic_pattern_matching(&pattern);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "匹配结果: " << match_count << " 个匹配" << std::endl;
    std::cout << "执行时间: " << duration.count() << " 微秒" << std::endl;
    
    // 获取执行统计信息
    auto stats = dynamic_hg.get_execution_statistics();
    std::cout << "\n执行统计:" << std::endl;
    for (const auto& pair : stats) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
}

void test_incremental_updates() {
    std::cout << "\n=== 测试增量更新 ===" << std::endl;
    
    // 创建初始动态超图
    DynamicHypergraph dynamic_hg;
    
    // 初始状态
    std::cout << "初始状态:" << std::endl;
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 第一轮更新
    std::cout << "\n第一轮更新..." << std::endl;
    dynamic_hg.insert_hyperedge(0, {0, 1, 2});
    dynamic_hg.insert_hyperedge(1, {1, 2, 3});
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 第二轮更新
    std::cout << "\n第二轮更新..." << std::endl;
    dynamic_hg.insert_hyperedge(2, {2, 3, 4});
    dynamic_hg.modify_hyperedge(0, {0, 1, 2, 5}); // 添加顶点5
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 第三轮更新
    std::cout << "\n第三轮更新..." << std::endl;
    dynamic_hg.delete_hyperedge(1);
    dynamic_hg.insert_vertex(6, {0, 2});
    std::cout << dynamic_hg.get_system_status() << std::endl;
    
    // 执行剪枝
    std::cout << "\n执行剪枝..." << std::endl;
    dynamic_hg.perform_pruning();
    std::cout << dynamic_hg.get_system_status() << std::endl;
}

int main() {
    std::cout << "动态超图匹配系统测试" << std::endl;
    std::cout << "====================" << std::endl;
    
    try {
        // 测试基本操作
        test_dynamic_hypergraph_basic_operations();
        
        // 测试从静态超图创建
        test_dynamic_hypergraph_from_static();
        
        // 测试动态模式匹配
        test_dynamic_pattern_matching();
        
        // 测试增量更新
        test_incremental_updates();
        
        std::cout << "\n所有测试完成！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "测试过程中发生错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}





