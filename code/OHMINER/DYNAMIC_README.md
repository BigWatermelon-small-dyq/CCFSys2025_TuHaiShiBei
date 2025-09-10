# OHMiner 动态超图匹配系统

## 概述

本系统在原有OHMiner静态超图匹配的基础上，实现了动态超图匹配功能。核心思想是根据时间的变化使用增量的思想对于系统中的各个部分进行调整，使得能够处理变化后的超图（相当于静态超图，处理过程和原论文一致）。

## 系统架构

### 1. 动态重叠索引 (Dynamic Overlap Index)
- **功能**: 快速获取某个超边集合的overlapping，并且能够在增删改事件下进行局部更新
- **核心特性**:
  - 插入超边时：计算新超边与现有超边之间新增的重叠关系，更新OIG，执行局部匹配检查
  - 删除超边时：识别并删除相关重叠关系，标记无效匹配实例
  - 修改超边时：看做删除后重新添加

### 2. 动态化的无冗余编译器 (Dynamic OIG)
- **功能**: 动态更新OIG，生成模式重叠序列和执行计划
- **核心特性**:
  - OIG初始化：根据初始静态超图构建OIG
  - 超边插入：更新OIG节点和边关系，保持分组关系
  - 超边删除：移除相关节点和边，更新分组
  - 结构更新：重新计算层级分组，生成执行序列和计划

### 3. 并行执行引擎 (Parallel Execution Engine)
- **功能**: 根据更新后的OIG和执行计划进行超图挖掘
- **核心特性**:
  - 部分匹配存储：维护所有部分匹配实例
  - 剪枝触发：插入时检查新重叠，删除时检查匹配有效性
  - 扩展部分匹配：避免完全重新计算

### 4. 动态化的度感知数据存储 (Degree-Aware Storage)
- **功能**: 使用散列表存储超边的度值分组
- **核心特性**:
  - 超边操作：插入、删除、修改超边时更新度值分组
  - 顶点操作：插入/删除顶点时更新相关超边的度值
  - 分组维护：保持度值分组的准确性和一致性

## 使用方法

### 基本操作

```cpp
#include "dynamic_hypergraph.h"

// 创建动态超图
DynamicHypergraph dynamic_hg;

// 插入超边
dynamic_hg.insert_hyperedge(0, {0, 1, 2});
dynamic_hg.insert_hyperedge(1, {1, 2, 3});

// 修改超边
dynamic_hg.modify_hyperedge(0, {0, 1, 2, 4});

// 删除超边
dynamic_hg.delete_hyperedge(1);

// 插入顶点
dynamic_hg.insert_vertex(5, {0, 2});

// 删除顶点
dynamic_hg.delete_vertex(3, {1, 2});
```

### 从静态超图创建

```cpp
#include "hypergraph.h"
#include "dynamic_hypergraph.h"

// 创建静态超图
hypergraph static_hg(4, 6);
// ... 设置超图数据 ...

// 创建动态超图
DynamicHypergraph dynamic_hg(&static_hg);
```

### 模式匹配

```cpp
#include "pattern.h"

// 创建模式
Pattern pattern;
// ... 设置模式数据 ...

// 执行动态模式匹配
int64_t match_count = dynamic_hg.dynamic_pattern_matching(&pattern);

// 执行并行动态模式匹配
int64_t match_count_parallel = dynamic_hg.dynamic_pattern_matching_parallel(&pattern);
```

### 系统监控

```cpp
// 获取系统状态
std::string status = dynamic_hg.get_system_status();
std::cout << status << std::endl;

// 获取执行统计
auto stats = dynamic_hg.get_execution_statistics();
for (const auto& pair : stats) {
    std::cout << pair.first << ": " << pair.second << std::endl;
}

// 执行剪枝操作
dynamic_hg.perform_pruning();
```

## API接口

### DynamicHypergraph 类

#### 构造函数
- `DynamicHypergraph()`: 创建空的动态超图
- `DynamicHypergraph(const hypergraph* g)`: 从静态超图创建动态超图

#### 动态操作接口
- `void insert_hyperedge(int hyperedge_id, const std::vector<int>& vertices)`: 插入超边
- `void delete_hyperedge(int hyperedge_id)`: 删除超边
- `void modify_hyperedge(int hyperedge_id, const std::vector<int>& new_vertices)`: 修改超边
- `void insert_vertex(int vertex_id, const std::vector<int>& incident_hyperedges)`: 插入顶点
- `void delete_vertex(int vertex_id, const std::vector<int>& incident_hyperedges)`: 删除顶点

#### 模式匹配接口
- `int64_t dynamic_pattern_matching(Pattern* p)`: 动态模式匹配
- `int64_t dynamic_pattern_matching_parallel(Pattern* p)`: 并行动态模式匹配

#### 系统管理接口
- `std::string get_system_status() const`: 获取系统状态
- `void perform_pruning()`: 执行剪枝操作
- `std::unordered_map<std::string, int64_t> get_execution_statistics() const`: 获取执行统计

### DynamicOverlapIndex 类

#### 重叠管理
- `void insert_hyperedge(int hyperedge_id, const std::vector<int>& vertices)`: 插入超边并更新重叠
- `void delete_hyperedge(int hyperedge_id)`: 删除超边并更新重叠
- `void modify_hyperedge(int hyperedge_id, const std::vector<int>& new_vertices)`: 修改超边并更新重叠

#### 查询接口
- `std::vector<int> get_hyperedge_overlaps(int hyperedge_id) const`: 获取超边的重叠关系
- `std::vector<int> get_vertex_overlaps(int vertex_id) const`: 获取顶点的重叠关系
- `bool is_overlap_valid(int overlap_id) const`: 检查重叠区域是否有效

### DynamicOIG 类

#### OIG管理
- `void initialize_from_hypergraph(const hypergraph* g)`: 从静态超图初始化OIG
- `void insert_hyperedge(int hyperedge_id, const std::vector<int>& vertices)`: 插入超边并更新OIG
- `void delete_hyperedge(int hyperedge_id)`: 删除超边并更新OIG
- `void update_structure()`: 更新OIG结构

#### 执行计划生成
- `std::vector<int> generate_execution_sequence() const`: 生成执行序列
- `std::vector<std::vector<int>> generate_execution_plan() const`: 生成执行计划

### PartialMatchStorage 类

#### 匹配管理
- `int add_partial_match(...)`: 添加部分匹配
- `void update_partial_match(...)`: 更新部分匹配
- `void invalidate_match(int match_id)`: 标记匹配为无效
- `void prune_invalid_matches()`: 剪枝无效匹配

### DegreeAwareStorage 类

#### 度值管理
- `void insert_hyperedge(...)`: 插入超边并更新度值分组
- `void delete_hyperedge(...)`: 删除超边并更新度值分组
- `void modify_hyperedge(...)`: 修改超边并更新度值分组
- `std::vector<int> get_hyperedges_by_degree(int degree) const`: 获取指定度值的超边

## 编译和运行

### 编译

```bash
cd OHMiner
mkdir build
cd build
cmake ..
make
```

### 运行测试

```bash
# 运行动态超图测试
./test/dynamic_hypergraph_test

# 运行原有测试
./test/overlap_match
./test/overlap_match_parallel
./test/random_sample
```

## 性能特点

### 增量更新优势
- **局部更新**: 只更新受影响的系统组件，避免全局重建
- **剪枝优化**: 利用部分匹配信息进行有效剪枝
- **并行处理**: 支持并行模式匹配，提高处理效率

### 内存管理
- **智能清理**: 自动清理无效的重叠区域和匹配实例
- **分组维护**: 保持度值分组的准确性和一致性
- **结构优化**: 动态维护OIG的层次结构

## 扩展和优化

### 当前实现的简化
- 重叠计算：当前使用简化版本，实际应实现精确的顶点交集计算
- 超图更新：基础超图的更新逻辑需要根据具体需求实现
- 模式匹配：当前直接调用原有算法，可进一步优化

### 未来优化方向
- 实现精确的重叠计算算法
- 优化剪枝策略，提高匹配效率
- 增强并行处理能力
- 添加缓存机制，减少重复计算

## 注意事项

1. **内存管理**: 动态操作会产生临时对象，注意及时清理
2. **一致性维护**: 确保各个组件之间的数据一致性
3. **性能监控**: 定期执行剪枝操作，保持系统性能
4. **错误处理**: 添加适当的错误检查和异常处理

## 联系和支持

如有问题或建议，请联系开发团队。





