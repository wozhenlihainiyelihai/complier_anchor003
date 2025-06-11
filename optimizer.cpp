#include "optimizer.h"
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <functional>
#include <set>
#include <list>

using namespace std;

// 检查字符串是否是数字
bool is_numeric(const std::string& s) {
    if (s.empty() || s.front() == '"' || s.front() == '\'') return false;
    char* end = nullptr;
    strtod(s.c_str(), &end); //从字符串开头开始解析，知道遇到无法解析的数字，讲字符地址存放在end中
    return end != s.c_str() && *end == '\0'; //当至少解析了一个字符或者只想末尾，则字符串被解析成数字
}

// 判断一个变量名是否为临时变量
bool is_temporary_var(const std::string& s) {
    if (s.length() > 1 && s[0] == 'T') {
        for (size_t i = 1; i < s.length(); ++i) {
            if (!isdigit(s[i])) return false;
        }
        return true;
    }
    return false;
}

// 判断一个字符串是否为变量
bool Optimizer::is_variable(const std::string& s) {
    if (s.empty() || s == "_") return false;
    return !is_numeric(s) && s.front() != '"' && s.front() != '\'';
}

// 构造函数
Optimizer::Optimizer(const std::vector<Quadruple>& quads, SymbolTable& st)
    : input_quads(quads), symbol_table(st) {}

// 主优化函数，协调所有步骤
vector<Quadruple> Optimizer::optimize() {
    if (input_quads.empty()) return {};

    // 步骤 1 & 2: 划分基本块、构建CFG和进行活跃变量分析
    const auto& all_symbols = symbol_table.getAllSymbols();//符号表中获取所有的符号
    for (const auto& [name, symbol] : all_symbols) {
        if (symbol.category == SymbolCategory::Variable && symbol.scopeLevel == 0) {
            this->globals.insert(name); //存放全局global变量
        }
    }

    divide_into_basic_blocks();
    cout << "--- 已将四元式划分为 " << basic_blocks.size() << " 个基本块 ---" << endl;

    build_cfg_and_compute_use_def();//构建控制流图，计算基本块的use和def集合
    run_liveness_analysis();//活跃变量分析

    // 步骤 3: 对每个基本块进行“原地”优化
    // 这个循环只负责调用优化，不产生最终列表
    for (auto& block : basic_blocks) {
        optimize_block(block);
    }

    // 步骤 4: 在所有块都优化完毕后，按原始顺序重新组装
    vector<Quadruple> assembled_quads;
    for (const auto& block : basic_blocks) {
        assembled_quads.insert(assembled_quads.end(), block.quads.begin(), block.quads.end());
    }

    // 步骤 5: 在组装好的、顺序正确的列表上进行死标签移除
    set<string> used_labels;
    for(const auto& q : assembled_quads) {
        if(q.op == "JUMP" || q.op == "JUMPF" || q.op == "JUMPNZ") {
            used_labels.insert(q.res);
        }
    }

    optimized_quads.clear();
    for(const auto& q : assembled_quads) {
        if(q.op == "LABEL" && used_labels.find(q.arg1) == used_labels.end()) {
            cout << "  [移除未使用标签] " << q.arg1 << endl;
            continue;
        }
        optimized_quads.push_back(q);
    }

    return optimized_quads;
}

// 1. 划分基本块
void Optimizer::divide_into_basic_blocks() {
    set<size_t> leaders; //无符号长整型
    leaders.insert(0);

    map<string, size_t> label_to_index;
    for (size_t i = 0; i < input_quads.size(); ++i) {
        if (input_quads[i].op == "LABEL") {
            label_to_index[input_quads[i].arg1] = i;
        }
    }

    for (size_t i = 0; i < input_quads.size(); ++i) {
        const auto& q = input_quads[i];
        string op = q.op;
        if (op == "JUMP" || op == "JUMPF" || op == "RETURN" || op == "CALL" || op == "FUNC_END") {
            if (op == "JUMP" || op == "JUMPF") {//如果是跳转指令，那就讲索引加入leaders集合
                if (label_to_index.count(q.res)) {
                    leaders.insert(label_to_index.at(q.res));
                }
            }
            if (i + 1 < input_quads.size()) {//如果接下来的指令也是入口，那就将索引加入leaders集合
                leaders.insert(i + 1);
            }
        }
    }

    auto it = leaders.begin();
    int block_id_counter = 0;
    while (it != leaders.end()) {
        size_t start_index = *it;
        auto next_it = std::next(it);
        size_t end_index = (next_it != leaders.end()) ? *next_it : input_quads.size();

        if (start_index < end_index) {
             vector<Quadruple> block_quads(input_quads.begin() + start_index, input_quads.begin() + end_index);
             basic_blocks.emplace_back(block_id_counter++, std::move(block_quads));
        }
        it = next_it;
    }
}

// 2. 构建CFG并计算use/def集：从后往前
void Optimizer::build_cfg_and_compute_use_def() {
    map<string, int> label_to_block_id; //label和id的映射表
    for (size_t i = 0; i < basic_blocks.size(); ++i) {
        if (!basic_blocks[i].quads.empty() && basic_blocks[i].quads[0].op == "LABEL") {
            label_to_block_id[basic_blocks[i].quads[0].arg1] = basic_blocks[i].id;
        }
    }

    for (size_t i = 0; i < basic_blocks.size(); ++i) {
        auto& block = basic_blocks[i];
        for (auto it = block.quads.rbegin(); it != block.quads.rend(); ++it) {
            const auto& q = *it;
            if (is_variable(q.res)) {//如果指令结果是一个变量，证明指令定义了变量
                block.use.erase(q.res);//如果指令存在，那就会被从use里抹掉
                block.def.insert(q.res);//并且在def集增加
            }
            if (is_variable(q.arg1)) block.use.insert(q.arg1);//如果操作数是变量，就证明被use了
            if (is_variable(q.arg2)) block.use.insert(q.arg2);//同理
        }

        if (block.quads.empty()) continue;
        const auto& last_quad = block.quads.back();
        if (last_quad.op == "JUMP") {
            if (label_to_block_id.count(last_quad.res)) { //跳转到唯一的后继
                block.successors.push_back(label_to_block_id.at(last_quad.res));
            }
        } else if (last_quad.op == "JUMPF") {
            if (label_to_block_id.count(last_quad.res)) {//要么跳转到目标的基本快
                block.successors.push_back(label_to_block_id.at(last_quad.res));
            }
            if (i + 1 < basic_blocks.size()) {//要么顺序执行下一个
                block.successors.push_back(basic_blocks[i+1].id);
            }
        } else if (last_quad.op != "RETURN" && last_quad.op != "FUNC_END") {
            if (i + 1 < basic_blocks.size()) {
                block.successors.push_back(basic_blocks[i+1].id);
            }
        }
    }
    //通过后继successors来填充predecessors
    for(size_t i = 0; i < basic_blocks.size(); ++i) {
        for(int successor_id : basic_blocks[i].successors) {
            for(auto& b : basic_blocks) {
                if(b.id == successor_id) {
                    b.predecessors.push_back(basic_blocks[i].id);
                    break;
                }
            }
        }
    }
}

// 3. 运行活跃变量分析
void Optimizer::run_liveness_analysis() {//迭代到集合不变为止
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = basic_blocks.size() - 1; i >= 0; --i) {
            auto& block = basic_blocks[i];
            set<string> new_out;
            for (int succ_id : block.successors) {
                 for(const auto& b : basic_blocks) {
                    if (b.id == succ_id) { //一个块的出口活跃变量是后继入口活跃变量集合
                         new_out.insert(b.live_in.begin(), b.live_in.end());
                         break;
                    }
                 }
            }

            if(new_out != block.live_out){
                block.live_out = new_out;
                changed = true;
            }

            set<string> new_in = block.use;
            set<string> out_minus_def = block.live_out;
            for (const auto& var : block.def) {
                out_minus_def.erase(var);
            }
            new_in.insert(out_minus_def.begin(), out_minus_def.end());

            if (new_in != block.live_in) {
                block.live_in = new_in;
                changed = true;
            }
        }
    }
}

// 4. 对单个基本块进行DAG优化
void Optimizer::optimize_block(BasicBlock& block) {
    cout << "\n--- 正在优化基本块 " << block.id << " (size=" << block.quads.size() << ") ---" << endl;
    if (block.quads.empty()) return;

    map<string, DagNode*> var_to_node;
    list<unique_ptr<DagNode>> all_nodes;
    int nodeIdCounter = 0;

    auto find_or_create_leaf = [&](const string& name) -> DagNode* {
        // 如果是变量。
        if (is_variable(name)) {
            // 如果这个变量已经在map中有关联的节点，直接返回该节点。
            if (var_to_node.count(name)) return var_to_node[name];
            // 如果是常量（数字或字符串字面量）。
        } else {
            // 遍历所有已创建的节点，看是否已有代表此常量的叶子节点。
            for (const auto& node : all_nodes) {
                if (node->op == "leaf" && !node->labels.empty() && node->labels[0] == name) return node.get();
            }
        }
        // 如果找不到，创建一个新的叶子节点。
        auto node = make_unique<DagNode>(nodeIdCounter++, "leaf");
        // 将变量名或常量值作为它的第一个标签。
        node->labels.push_back(name);
        DagNode* ptr = node.get(); // 获取原始指针。
        all_nodes.push_back(std::move(node)); // 将新节点存入列表中。
        // 如果是变量，更新map，建立关联。
        if (is_variable(name)) var_to_node[name] = ptr;
        return ptr;
    };

    for(const auto& live_var : block.live_in) find_or_create_leaf(live_var);

    // 第一步: 构建DAG
    vector<Quadruple> side_effect_quads;
    for (const auto& q : block.quads) {
        bool is_expr = (q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/" || q.op == ">" || q.op == "<" || q.op == "==" || q.op == "!=" || q.op == "&&" || q.op == "||");

        if (is_expr) {
            //为左右操作数查找或者创建dag节点
            DagNode* left = find_or_create_leaf(q.arg1);
            DagNode* right = find_or_create_leaf(q.arg2);
            //常量折叠直接算
            if (is_numeric(left->labels.front()) && is_numeric(right->labels.front()) &&
               (q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/"))
            {
                 double v1 = stod(left->labels.front());//字符串转换成double类型
                 double v2 = stod(right->labels.front());
                 double res_v = 0;

                 if (q.op == "+") res_v = v1 + v2;
                 else if (q.op == "-") res_v = v1 - v2;
                 else if (q.op == "*") res_v = v1 * v2;
                 else if (q.op == "/" && v2 != 0) res_v = v1 / v2;
                 else goto perform_cse; // 除以零或其他情况，跳转

                 stringstream ss; ss << res_v;//变回字符串

                 if (var_to_node.count(q.res)) {//移除旧的关联
                    auto& labels = var_to_node[q.res]->labels;
                    labels.erase(remove(labels.begin(), labels.end(), q.res), labels.end());
                 }
                 var_to_node[q.res] = find_or_create_leaf(ss.str());//new一个
                 cout << "  [常量折叠] " << q.toString() << " -> " << ss.str() << endl;
                 continue;
            }

        perform_cse://公共子表达式消除
            DagNode* existing_node = nullptr;
            for(const auto& node_ptr : all_nodes) {
                if(node_ptr->op != "leaf" && node_ptr->equals(q.op, left, right)) {//是否存在完全相同的计算node
                    existing_node = node_ptr.get(); break;
                }
            }
            if (var_to_node.count(q.res)) {//将目标变量从旧node移除
                auto& labels = var_to_node[q.res]->labels;
                labels.erase(remove(labels.begin(), labels.end(), q.res), labels.end());
            }

            // 如果真有
            if (existing_node) {
                cout << "  [CSE] " << q.toString() << endl;
                // 直接将当前指令的目标变量作为新标签添加到这个已存在节点上。
                existing_node->labels.push_back(q.res);
                // 更新map，将目标变量关联到这个节点。
                var_to_node[q.res] = existing_node;
                // 如果没找着
            } else {
                // 创建一个新的内部节点来代表这个计算。
                auto new_node = make_unique<DagNode>(nodeIdCounter++, q.op);
                new_node->left = left; new_node->right = right;
                new_node->labels.push_back(q.res);
                // 更新map，将目标变量关联到这个新节点。
                var_to_node[q.res] = new_node.get();
                all_nodes.push_back(std::move(new_node));
            }

        } else if (q.op == "=") {
            // 对全局变量的赋值是一种副作用，必须保留，不能动弹
            if (globals.count(q.res)) {
                side_effect_quads.push_back(q);
            }
            // 无论如何，都需要更新DAG
            if (var_to_node.count(q.res)) {
                auto& labels = var_to_node[q.res]->labels;
                labels.erase(remove(labels.begin(), labels.end(), q.res), labels.end());
            }
            DagNode* arg1_node = find_or_create_leaf(q.arg1);
            arg1_node->labels.push_back(q.res);
            var_to_node[q.res] = arg1_node;

        } else {
            // 所有其他类型的指令，比如call jump等等，它们被视为有副作用，不能被优化掉，直接存起来。
            side_effect_quads.push_back(q);
            // 如果是函数调用，它可能会修改任何全局变量或通过指针修改内存。
            // 为了安全起见，我们假设所有全局变量的值都可能已失效。
            if (q.op == "CALL") {
                // 如果调用有返回值，为返回值创建一个新的叶子节点，表示它的值是全新的。
                if (is_variable(q.res)) var_to_node[q.res] = find_or_create_leaf(q.res);
                // 同样，为所有全局变量创建新的叶子节点，表示它们的值可能已被修改。
                for (const auto& g : globals) var_to_node[g] = find_or_create_leaf(g);
            }
        }
    }

    // 第二步: 识别所有必需的节点
    set<DagNode*> needed_nodes;
    list<DagNode*> worklist;

    // 根节点：出口活跃变量 和 有副作用指令使用的变量
    for (const auto& live_var : block.live_out) {
        if (var_to_node.count(live_var)) { //还没标记为必须那就标记为必须
            if(needed_nodes.find(var_to_node[live_var]) == needed_nodes.end()){
                worklist.push_back(var_to_node[live_var]);
                needed_nodes.insert(var_to_node[live_var]);
            }
        }
    }
    for (const auto& q : side_effect_quads) {//是副作用的话，节点也是必须的
        if (is_variable(q.arg1) && var_to_node.count(q.arg1)) {
            if(needed_nodes.find(var_to_node[q.arg1]) == needed_nodes.end()){
                 worklist.push_back(var_to_node[q.arg1]);
                 needed_nodes.insert(var_to_node[q.arg1]);
            }
        }
        if (is_variable(q.arg2) && var_to_node.count(q.arg2)) {
             if(needed_nodes.find(var_to_node[q.arg2]) == needed_nodes.end()){
                 worklist.push_back(var_to_node[q.arg2]);
                 needed_nodes.insert(var_to_node[q.arg2]);
            }
        }
        if (is_variable(q.res) && var_to_node.count(q.res)) {
             if(needed_nodes.find(var_to_node[q.res]) == needed_nodes.end()){
                 worklist.push_back(var_to_node[q.res]);
                 needed_nodes.insert(var_to_node[q.res]);
            }
        }
    }

    // 反向追溯所有依赖的节点
    while(!worklist.empty()){
        DagNode* node = worklist.front();
        worklist.pop_front();
        if(node->left && needed_nodes.find(node->left) == needed_nodes.end()){ //当前节点的子节点也是必须的
            needed_nodes.insert(node->left);
            worklist.push_back(node->left);
        }
        if(node->right && needed_nodes.find(node->right) == needed_nodes.end()){
            needed_nodes.insert(node->right);
            worklist.push_back(node->right);
        }
    }

    // 第三步: 从DAG生成代码
    vector<Quadruple> final_block_code;//存放新生成的优化代码
    set<int> generated_node_ids;
    //用于便利dag并生成代码
    function<void(DagNode*)> generate_code =
        [&](DagNode* node) {
        if (!node || generated_node_ids.count(node->id)) return;//空or处理过

        if (needed_nodes.find(node) == needed_nodes.end()) { //非必须
             if (node->op != "leaf") {
                 cout << "  [死代码消除] " << Quadruple(node->op, node->left->labels.front(), node->right ? node->right->labels.front() : "_", node->labels.front()).toString() << endl;
             }
             return;
        }
        //后序遍历生成子节点代码
        generate_code(node->left);
        generate_code(node->right);

        if (node->op == "leaf") {
            generated_node_ids.insert(node->id);
            return;
        }

        string primary_label = node->labels.front();
        for(const auto& label : node->labels) {
            if(!is_temporary_var(label)) {
                primary_label = label; //优先使用用户定义名，而不是临时变量
                break;
            }
        }

        string arg1_val = node->left->labels.front();
        string arg2_val = node->right ? node->right->labels.front() : "_";

        final_block_code.emplace_back(node->op, arg1_val, arg2_val, primary_label);
        cout << "  [生成] " << final_block_code.back().toString() << endl;
        //更新dag，确保主标签在最前
        var_to_node[primary_label] = node;
        node->labels.insert(node->labels.begin(), primary_label);

        generated_node_ids.insert(node->id);//标记已经处理的节点
    };

    for(const auto& node_ptr : all_nodes){
        generate_code(node_ptr.get());//调用代码生成函数
    }

    // 为出口活跃变量生成最终赋值
    for (const auto& live_var : block.live_out) {
        if (!var_to_node.count(live_var)) continue;
        // 如果是全局变量，它的赋值已经作为副作用处理，这里不再生成
        if (globals.count(live_var)) continue;

        string current_val = var_to_node[live_var]->labels.front();
        if (live_var != current_val) {//名字不同的，需要再生成一条语句哦
            final_block_code.emplace_back("=", current_val, "_", live_var);
        }
    }

    // 添加有副作用的指令
    for(auto& q : side_effect_quads){
        // 在添加前，将其操作数更新为优化后的最新值（即其在DAG中对应节点的标签）。
        if(is_variable(q.arg1) && var_to_node.count(q.arg1)) q.arg1 = var_to_node.at(q.arg1)->labels.front();
        if(is_variable(q.arg2) && var_to_node.count(q.arg2)) q.arg2 = var_to_node.at(q.arg2)->labels.front();
    }
    // 将更新后的副作用指令追加到代码末尾。
    final_block_code.insert(final_block_code.end(), side_effect_quads.begin(), side_effect_quads.end());

    cout << "--- 优化后基本块 (size=" << final_block_code.size() << ") ---" << endl;
    // 最后，用新生成的、优化过的代码，替换掉基本块中的旧代码。
    block.quads = final_block_code;
}
