#pragma once
#include<vector>
#include "map"
#include "unordered_map"
#include "common.h"
#include "string"
#include "pattern.h"
#include "set"
#include "unordered_set"

class Pattern;
struct Stable{
public:
    std::vector<int> hedge_list;
    std::unordered_map<int, std::vector<int>> sedge;                      //sedge表示的是一系列的超边
    std::unordered_map<int, std::vector<int>> invert_index;              //倒排索引
};

class hypergraph{
public:
    //没有hyperedge 包含超边的信息啊
    hypergraph(int e_cnt, int v_cnt);
    int e_cnt;
    int v_cnt;
    std::vector<int>hedge_to_kid;
    std::vector<int>v_to_lable;
    std::unordered_map<int , Stable> HyperGraph_table;


    //hyperedge2node、node2hyperedge、hyperedge_adj是为了random_walk
    std::vector< std::vector<int> >hyperedge2node;        //the hyperedge of the query
    std::vector< std::vector<int> >node2hyperedge;        //表示的是node 和 hyperedge之间的关系
    std::vector< std::vector<int> > hyperedge_adj;        //表示的是超边之间的连接关系

    std::vector< std::vector<int> >csr;
    std::vector< std::vector<int> >deg;
    //std::vector< std::unordered_map<int, std::vector<std::pair<int, int>>> > hyperedgeAdj;
    std::vector< std::unordered_map<int, std::vector<int> >> hyperedgeAdj;

    std::unordered_map<int64_t, std::vector<int> > hyper_inter;


    // int max_size_DAG = 0;
    std::vector<std::vector<int>> DAG;
    std::vector<std::vector<std::vector<int>>> DAG_vector;
    static double valid_time;
    static double generate_time;
    static double intersect_time;
    static long long candidate_num;

    int64_t overlap_aware_pattern_matching(Pattern* p);
    int64_t overlap_aware_pattern_matching_parallel(Pattern* p);
    int64_t overlap_aware_pattern_matching_parallel_openmp(Pattern *p);

    void Enumerate_overlap(std::vector<int>&matching_order, std::vector<int>& candidate, int depth, Pattern* p, std::vector<int>& subtraction_set, int64_t& count);
    void Enumerate_overlap_parallel(int thread_id, std::vector<int>&matching_order, std::vector<int>& candidate, int depth, Pattern* p, std::vector<int>& subtraction_set,std::vector<std::vector<int>> &DAG_temp);
    void Enumerate_overlap_parallel_openmp(int thread_id, std::vector<int> &matching_order, std::vector<int> &candidate, int depth , Pattern *p, std::vector<int> &subtraction_set,std::vector<std::vector<int>>& DAG_temp);
    
    void get_no_incident(int query_edge, std::vector<int>& v_no_incident, std::vector<int>& matching_order, std::vector<int>& subtraction_set, Pattern *p);
    void get_incident(int u, std::vector<int>&v_no_incident, int hedge, std::vector<int>& v_incident, Pattern* p, std::vector<int>& subtraction_set);

    bool isValidEmbedding_overlap(std::vector<int>&subtrction_set, std::vector<int>&matching_order,int depth, Pattern* p);
    bool isValidEmbedding_overlap_old(std::vector<int>&subtrction_set, std::vector<int>&matching_order,int depth, Pattern* p);
    bool isValidEmbedding_overlap_parallel(std::vector<int>&subtrction_set, std::vector<int>&matching_order,int depth, Pattern* p,std::vector<std::vector<int>> &DAG_temp);
    bool isValidEmbedding_overlap_parallel_old(std::vector<int>&subtrction_set, std::vector<int>&matching_order,int depth, Pattern* p,std::vector<std::vector<int>> &DAG_temp);

    void generate_hyperedge_candidates_overlap(Pattern* p, std::vector<int>& matching_order, int query_edge, std::vector<int>& subtraction_set, std::vector<int>& candidate);
    void generate_hyperedge_candidates_unlable(Pattern* p, std::vector<int>& matching_order, int query_edge, std::vector<int>& subtraction_set, std::vector<int>& candidate);
    
    void random_walk(int num_edges_to_sample, int min_vertex_count, int max_vertex_count, const char* pattern_path, const char* node_lable_path);
    void random_walk_dense(int num_edges_to_sample, int min_vertex_count, int max_vertex_count, const char* pattern_path, const char* node_lable_path);

    // 动态更新接口：新增顶点与超边
    // 返回新顶点ID
    int add_vertex(int vertex_label);
    // 返回新超边ID，输入顶点列表（允许无序、包含重复，内部会去重排序）
    int add_hyperedge(const std::vector<int>& vertices);

    // 供增量构建使用：查询/注册超边标签键到kid的映射
    // 键为按升序排列的顶点label序列
    int get_or_assign_kid(const std::vector<int>& edge_label_key);

    // 记录edge label key到kid的映射，支持动态新增
    std::map<std::vector<int>, int> key_to_kid;

    // 动态删除与修改接口
    // 删除整条超边（软删除：清空该边的顶点并从索引中移除），保持ID稳定
    void delete_hyperedge(int hyperedge_id);
    // 将超边的顶点集合替换为 new_vertices（内部去重排序），并维护所有元数据
    void modify_hyperedge_vertices(int hyperedge_id, const std::vector<int>& new_vertices);
    // 将顶点从所有关联超边中移除，并清空其 incident 列表
    void delete_vertex(int vertex_id);
    // 修改顶点标签，并更新受影响超边的标签kid与索引
    void set_vertex_label(int vertex_id, int new_label);

    // 在指定超边中增删单个顶点
    void add_vertex_to_hyperedge(int hyperedge_id, int vertex_id);
    void remove_vertex_from_hyperedge(int hyperedge_id, int vertex_id);

private:
    // 重建基于当前 hyperedge2node 与 node2hyperedge 的邻接、桶、交集、CSR/DEG
    void rebuild_connectivity();
    // 用新的顶点集合更新指定超边的 kid、sedge、hedge_list、invert_index
    void update_edge_tables(int hyperedge_id, const std::vector<int>& old_vertices, const std::vector<int>& new_vertices);
};


