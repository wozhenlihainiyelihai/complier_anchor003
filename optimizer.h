#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <set>
#include "quadruple.h"
#include "symbol_table.h"

// DAG中的节点
struct DagNode {
    int id;                                     // 节点的唯一ID
    std::string op;                             // 节点的操作符 (如 "+", "leaf")
    DagNode *left = nullptr, *right = nullptr;  // 指向左右子节点的指针
    std::vector<std::string> labels;            // 附加到此节点的变量名/临时变量名列表

    DagNode(int i, std::string o) : id(i), op(std::move(o)) {}

    // 比较两个节点是否等价（操作符和子节点都相同）
    bool equals(const std::string& other_op, DagNode* other_left, DagNode* other_right) const {
        return op == other_op && left == other_left && right == other_right;
    }
};

// 用于表示基本块及其数据流信息的结构
struct BasicBlock {
    int id;
    std::vector<Quadruple> quads;
    std::set<std::string> use;
    std::set<std::string> def;
    std::set<std::string> live_in;
    std::set<std::string> live_out;
    std::vector<int> successors;
    std::vector<int> predecessors;

    BasicBlock(int i, std::vector<Quadruple> q) : id(i), quads(std::move(q)) {}
};


// 优化器类
class Optimizer {
private:
    const std::vector<Quadruple>& input_quads;
    std::vector<Quadruple> optimized_quads;
    SymbolTable& symbol_table;
    std::vector<BasicBlock> basic_blocks; // 存储所有基本块
    std::set<std::string> globals;

    // 1. 将四元式序列划分为基本块
    void divide_into_basic_blocks();

    // 2. 构建CFG并计算use/def集
    void build_cfg_and_compute_use_def();

    // 3. 运行活跃变量分析
    void run_liveness_analysis();

    // 4. 对单个基本块进行DAG优化
    void optimize_block(BasicBlock& block);

    // 辅助函数：判断字符串是否为变量
    bool is_variable(const std::string& s);


public:
    Optimizer(const std::vector<Quadruple>& quads, SymbolTable& st);

    // 执行优化的主函数
    std::vector<Quadruple> optimize();
};

#endif // OPTIMIZER_H
