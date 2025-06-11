#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <map>

#include "quadruple.h"
#include "symbol_table.h"

// 描述栈上一个变量或参数的位置
struct StackLocation {
    int offset; // 相对于 BP 的偏移量
    int size;   // 变量大小 (例如 dw 是 2)
};

class CodeGenerator {
private:
    const std::vector<Quadruple>& quadruples;
    SymbolTable& symbolTable;
    std::stringstream assembly_code;

    // 状态管理
    std::string current_function; // 当前正在生成的函数名
    // 存储每个函数内所有局部变量和临时的位置映射
    std::map<std::string, std::unordered_map<std::string, StackLocation>> function_frames_layout;
    // 专门用于存储每个函数预计算好的局部变量总大小
    std::map<std::string, int> function_local_sizes;

    // 用于处理字符串字面量
    std::map<std::string, std::string> string_literals;
    int string_literal_counter = 0;

    // 代码生成阶段
    void preprocess_data();      // 预处理四元式，收集数据（如字符串、函数栈帧大小）
    void generateDataSegment();  // 生成 .DATA 数据段
    void generateCodeSegment();  // 生成 .CODE 代码段

    // 指令生成辅助函数
    void generateForQuad(const Quadruple& q);                      // 为单个四元式生成代码
    std::string getOperandAddress(const std::string& operand);     // 获取操作数的有效地址字符串
    std::shared_ptr<TypeInfo> getOperandType(const std::string& operand); // 获取操作数的类型信息
    void emit(const std::string& instruction, const std::string& comment = ""); // 发射单条汇编指令

    // 具体指令的处理函数
    void handleFunctionBegin(const Quadruple& q);
    void handleFunctionEnd(const Quadruple& q);
    void handleReturn(const Quadruple& q);
    void handleParam(const Quadruple& q);
    void handleCall(const Quadruple& q);
    void handlePrint(const Quadruple& q);
    void handleComparison(const Quadruple& q);
    void handleStringConcat(const Quadruple& q); //处理字符串拼接

    // 数组操作处理函数
    void handleArrayDeclaration(const Quadruple& q);
    void handleStoreAt(const Quadruple& q);
    void handleLoadAt(const Quadruple& q);

    // 占位符
    void handleGetParam(const Quadruple& q);


public:
    CodeGenerator(const std::vector<Quadruple>& quads, SymbolTable& st);
    std::string generate(); // 生成汇编代码的公共接口
};

#endif // CODE_GENERATOR_H
