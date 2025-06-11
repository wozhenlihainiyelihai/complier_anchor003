#include "code_generator.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <set>

using namespace std;

// 构造函数
CodeGenerator::CodeGenerator(const std::vector<Quadruple>& quads, SymbolTable& st)
    : symbolTable(st), quadruples(quads), string_literal_counter(0) {}

// 主生成函数，协调所有步骤
string CodeGenerator::generate() {
    assembly_code.str(""); // 清空旧内容
    preprocess_data();

    assembly_code << ".MODEL SMALL" << endl;
    assembly_code << ".STACK 200h" << endl;

    generateDataSegment();
    generateCodeSegment();

    return assembly_code.str();
}

// 预处理四元式，为数据段和代码段的生成做准备
void CodeGenerator::preprocess_data() {
    // 第一遍：收集所有字符串字面量
    for (const auto& q : quadruples) {
        // 字符串可能出现在 PRINT, =, + 等多种操作中
        const string* ops[] = {&q.arg1, &q.arg2, &q.res};
        for(const auto* op : ops) {
            if (op && op->length() > 1 && op->front() == '"') {
                if (string_literals.find(*op) == string_literals.end()) {
                    string label = "LC" + to_string(string_literal_counter++);
                    string_literals[*op] = label;
                }
            }
        }
    }

    // 第二遍：为每个函数计算栈帧布局和大小
    string active_function_name;
    for (const auto& q_func_start : quadruples) {
        if (q_func_start.op == "FUNC_BEGIN") {
            active_function_name = q_func_start.arg1;
            auto& current_layout = function_frames_layout[active_function_name];
            int current_local_offset = 0;

            // 扫描函数体内的所有四元式，确定所有局部变量和临时变量
            bool in_func_body = false;
            for (const auto& q_inner : quadruples) {
                if (q_inner.op == "FUNC_BEGIN" && q_inner.arg1 == active_function_name) {
                    in_func_body = true;
                }
                if (!in_func_body) continue;
                if (q_inner.op == "FUNC_END" && q_inner.arg1 == active_function_name) break;

                const string* operands[] = {&q_inner.arg1, &q_inner.arg2, &q_inner.res};
                for (const auto* op_ptr : operands) {
                    const string& op_name = *op_ptr;
                    if (op_name.empty() || op_name == "_" || isdigit(op_name[0]) || op_name.front() == '"' || op_name.front() == '\'') continue;

                    const Symbol* sym = symbolTable.lookup(op_name);
                    if(sym && sym->scopeLevel == 0) continue; // 全局变量，不在栈上

                    bool is_param = false;
                    const Symbol* func_sym = symbolTable.lookup(active_function_name);
                    if(func_sym) for(const auto& p : func_sym->type->parameters) if(p.name == op_name) is_param = true;

                    // 如果不是参数，并且尚未分配，则在栈上为其分配空间
                    if (!is_param && current_layout.find(op_name) == current_layout.end()) {
                        int size_to_alloc = 2; // 默认为 WORD
                         if ((q_inner.op == "DEC_ARRAY" || q_inner.op == "DEC_DYN_ARRAY") && q_inner.arg1 == op_name) {
                            try {
                                size_to_alloc = stoi(q_inner.arg2) * 2; // 静态数组
                            } catch(...) { size_to_alloc = 2; } // 动态数组本身只存指针
                         }
                        current_local_offset += size_to_alloc;
                        current_layout[op_name] = {-current_local_offset, size_to_alloc};
                    }
                }
            }
            function_local_sizes[active_function_name] = current_local_offset;
        }
    }
}


// 生成 .DATA 数据段
void CodeGenerator::generateDataSegment() {
    assembly_code << "\n.DATA" << endl;
    assembly_code << "    int_fmt db \"%d\", 10, 0" << endl;
    assembly_code << "    str_fmt db \"%s\", 10, 0" << endl;

    // 新增: 为字符串操作添加缓冲区
    assembly_code << "    int_str_buffer db 12 dup(0)      ; 用于 _itoa 转换整数为字符串" << endl;
    assembly_code << "    concat_buffer db 256 dup(0)     ; 用于字符串拼接的结果" << endl;

    for (const auto& pair : string_literals) {
        string sanitized_str = pair.first.substr(1, pair.first.length() - 2);
        // 处理转义字符，例如 `\n`
        string final_str;
        for(size_t i = 0; i < sanitized_str.length(); ++i) {
            if (sanitized_str[i] == '\\' && i + 1 < sanitized_str.length()) {
                if (sanitized_str[i+1] == 'n') {
                    final_str += "\", 10, \""; // 换行
                    i++;
                } else {
                    final_str += sanitized_str[i]; // 其他转义字符暂不处理
                }
            } else {
                final_str += sanitized_str[i];
            }
        }
        assembly_code << "    " << pair.second << " db \"" << final_str << "\", 0" << endl;
    }

    // 为全局变量分配空间
    const auto& symbols = symbolTable.getAllSymbols();
    for (const auto& pair : symbols) {
        const auto& sym = pair.second;
        if (sym.category == SymbolCategory::Variable && sym.scopeLevel == 0) {
             assembly_code << "    " << sym.name << " dw ?" << endl;
        }
    }
}

// 生成 .CODE 代码段
void CodeGenerator::generateCodeSegment() {
    assembly_code << "\n.CODE" << endl;
    // 新增: 声明需要用到的C库函数
    assembly_code << "EXTERN _printf : NEAR, _itoa : NEAR, _strcpy : NEAR, _strcat : NEAR" << endl;

    assembly_code << "\nmain PROC" << endl;
    emit("mov ax, @data", "设置数据段寄存器");
    emit("mov ds, ax");
    emit("call anchor_main", "调用我们语言的入口函数");
    emit("mov ah, 4Ch", "DOS退出程序功能");
    emit("int 21h");
    assembly_code << "main ENDP" << endl;

    for (const auto& quad : quadruples) {
        generateForQuad(quad);
    }

    assembly_code << "\nEND main" << endl;
}


// 新增: 获取操作数的类型信息
shared_ptr<TypeInfo> CodeGenerator::getOperandType(const string& operand) {
    if (operand.empty() || operand == "_") return symbolTable.lookupType("void");
    if (isdigit(operand[0]) || (operand.length() > 1 && operand[0] == '-')) return symbolTable.lookupType("int");
    if (operand == "true" || operand == "false") return symbolTable.lookupType("bool");
    if (operand.front() == '"') return symbolTable.lookupType("string");

    const Symbol* sym = symbolTable.lookup(operand);
    if (sym) return sym->type;

    return nullptr; // 未知类型
}

// 为单个四元式生成代码，这是一个总的分发器
void CodeGenerator::generateForQuad(const Quadruple& q) {
    assembly_code << "\n    ; " << q.toString() << endl;

    if (q.op == "=") {
        emit("mov ax, " + getOperandAddress(q.arg1));
        emit("mov " + getOperandAddress(q.res) + ", ax");
    } else if (q.op == "+") {
        // 进行类型派发，判断是整数加法还是字符串拼接
        auto type1 = getOperandType(q.arg1);
        auto type2 = getOperandType(q.arg2);

        if (type1 && type2 && (type1->name == "string" || type2->name == "string")) {
            handleStringConcat(q);
        } else {
            // 默认进行整数加法
            emit("mov ax, " + getOperandAddress(q.arg1));
            emit("add ax, " + getOperandAddress(q.arg2));
            emit("mov " + getOperandAddress(q.res) + ", ax");
        }
    } else if (q.op == "-") {
        emit("mov ax, " + getOperandAddress(q.arg1));
        emit("sub ax, " + getOperandAddress(q.arg2));
        emit("mov " + getOperandAddress(q.res) + ", ax");
    }
    else if (q.op == "*") {
        emit("mov ax, " + getOperandAddress(q.arg1));
        emit("mov bx, " + getOperandAddress(q.arg2));
        emit("imul bx");
        emit("mov " + getOperandAddress(q.res) + ", ax");
    } else if (q.op == "/") {
        emit("mov ax, " + getOperandAddress(q.arg1));
        emit("cwd", "将AX的符号扩展到DX");
        emit("mov bx, " + getOperandAddress(q.arg2));
        emit("idiv bx");
        emit("mov " + getOperandAddress(q.res) + ", ax");
    } else if (q.op == "&&") {
        string false_label = symbolTable.generateLabel();
        string end_label = symbolTable.generateLabel();
        emit("mov ax, " + getOperandAddress(q.arg1));
        emit("cmp ax, 0");
        emit("je " + false_label);
        emit("mov ax, " + getOperandAddress(q.arg2));
        emit("cmp ax, 0");
        emit("je " + false_label);
        emit("mov " + getOperandAddress(q.res) + ", 1");
        emit("jmp " + end_label);
        assembly_code << false_label << ":" << endl;
        emit("mov " + getOperandAddress(q.res) + ", 0");
        assembly_code << end_label << ":" << endl;
    } else if (q.op == "<" || q.op == ">" || q.op == "==" || q.op == "!=" || q.op == ">=" || q.op == "<=") {
        handleComparison(q);
    } else if (q.op == "LABEL") {
        assembly_code << q.arg1 << ":" << endl;
    } else if (q.op == "JUMP") {
        emit("jmp " + q.res);
    } else if (q.op == "JUMPF") {
        emit("mov ax, " + getOperandAddress(q.arg1));
        emit("cmp ax, 0");
        emit("je " + q.res);
    }
    else if (q.op == "FUNC_BEGIN") handleFunctionBegin(q);
    else if (q.op == "FUNC_END")   handleFunctionEnd(q);
    else if (q.op == "PARAM")      handleParam(q);
    else if (q.op == "CALL")       handleCall(q);
    else if (q.op == "RETURN")     handleReturn(q);
    else if (q.op == "PRINT")      handlePrint(q);
    else if (q.op == "DEC_ARRAY" || q.op == "DEC_DYN_ARRAY")  handleArrayDeclaration(q);
    else if (q.op == "STORE_AT")   handleStoreAt(q);
    else if (q.op == "LOAD_AT")    handleLoadAt(q);
    else if (q.op == "GET_PARAM")  handleGetParam(q);
    else if (q.op == "LOAD_MEMBER") {
        // 四元式: (LOAD_MEMBER, dest, base, offset)
        // dest = q.arg1, base = q.arg2, offset = q.res
        emit("lea si, " + getOperandAddress(q.arg2), "获取结构体基地址到 SI");
        emit("add si, " + q.res, "加上成员偏移量");
        emit("mov ax, [si]", "从计算出的地址加载成员的值到 AX");
        emit("mov " + getOperandAddress(q.arg1) + ", ax", "将值存入目标变量");
    }
    else if (q.op == "STORE_MEMBER") {
        // 四元式: (STORE_MEMBER, src, base, offset)
        // src = q.arg1, base = q.arg2, offset = q.res
        emit("lea si, " + getOperandAddress(q.arg2), "获取结构体基地址到 SI");
        emit("add si, " + q.res, "加上成员偏移量");
        emit("mov ax, " + getOperandAddress(q.arg1), "获取要存储的源值到 AX");
        emit("mov [si], ax", "将值存入计算出的内存地址");
    }
    else {
        assembly_code << "    ; 未处理的操作: " << q.op << endl;
    }
}

// 处理字符串拼接
void CodeGenerator::handleStringConcat(const Quadruple& q) {
    auto type1 = getOperandType(q.arg1);
    auto type2 = getOperandType(q.arg2);

    // --- 准备第一个操作数 op1 ---
    if (type1 && type1->name == "string") {
        // op1 是字符串，直接用 strcpy 复制到 concat_buffer
        emit("push " + getOperandAddress(q.arg1), "压入源字符串地址");
        emit("push OFFSET concat_buffer", "压入目标缓冲区地址");
        emit("call _strcpy");
        emit("add sp, 4", "恢复栈");
    } else {
        // op1 是数字，先用 _itoa 转换，再用 strcpy 复制
        emit("push 10", "压入基数10");
        emit("push OFFSET int_str_buffer", "压入itoa的目标缓冲区");
        emit("push " + getOperandAddress(q.arg1), "压入要转换的整数");
        emit("call _itoa");
        emit("add sp, 6", "恢复栈");
        // 将转换后的字符串复制到主拼接缓冲区
        emit("push OFFSET int_str_buffer");
        emit("push OFFSET concat_buffer");
        emit("call _strcpy");
        emit("add sp, 4");
    }

    // --- 准备第二个操作数 op2，并与 concat_buffer 拼接 ---
    if (type2 && type2->name == "string") {
        // op2 是字符串，直接用 strcat 拼接到 concat_buffer
        emit("push " + getOperandAddress(q.arg2), "压入要拼接的字符串地址");
        emit("push OFFSET concat_buffer", "压入目标缓冲区地址");
        emit("call _strcat");
        emit("add sp, 4", "恢复栈");
    } else {
        // op2 是数字，先用 _itoa 转换，再用 strcat 拼接
        emit("push 10");
        emit("push OFFSET int_str_buffer");
        emit("push " + getOperandAddress(q.arg2));
        emit("call _itoa");
        emit("add sp, 6");
        // 将转换后的字符串拼接到主拼接缓冲区
        emit("push OFFSET int_str_buffer");
        emit("push OFFSET concat_buffer");
        emit("call _strcat");
        emit("add sp, 4");
    }

    // --- 结果 ---
    // 将最终拼接好的字符串 (位于concat_buffer) 的地址存入结果变量
    emit("mov ax, OFFSET concat_buffer");
    emit("mov " + getOperandAddress(q.res) + ", ax");
}

// 获取操作数的有效地址字符串
string CodeGenerator::getOperandAddress(const std::string& operand) {
    if (operand.empty() || operand == "_") return "";
    if (isdigit(operand[0]) || (operand.length() > 1 && operand[0] == '-')) return operand;
    if (operand == "true") return "1";
    if (operand == "false") return "0";
    if (string_literals.count(operand)) return "OFFSET " + string_literals.at(operand);

    // 局部变量或临时变量
    if (!current_function.empty() && function_frames_layout.count(current_function) && function_frames_layout.at(current_function).count(operand)) {
        return "WORD PTR [bp" + to_string(function_frames_layout.at(current_function).at(operand).offset) + "]";
    }

    // 参数
    const Symbol* func_sym = symbolTable.lookup(current_function);
    if (func_sym && func_sym->type->kind == TypeKind::FUNCTION) {
        int param_offset = 4; // BP(2) + RET(2)
        for (int i = func_sym->type->parameters.size() - 1; i >= 0; --i) {
            if (func_sym->type->parameters[i].name == operand) {
                return "WORD PTR [bp + " + to_string(param_offset) + "]";
            }
            param_offset += 2; // WORD size
        }
    }
    return "WORD PTR " + operand; // 全局变量
}

// 处理函数开始
void CodeGenerator::handleFunctionBegin(const Quadruple& q) {
    current_function = q.arg1;
    string proc_name = (q.arg1 == "main") ? "anchor_main" : q.arg1;
    assembly_code << "\n" << proc_name << " PROC" << endl;

    emit("push bp", "保存旧的基址指针");
    emit("mov bp, sp", "设置新的基址指针");
    if (function_local_sizes.count(current_function)) {
        int total_local_size = function_local_sizes.at(current_function);
        if (total_local_size > 0) {
            emit("sub sp, " + to_string(total_local_size), "为局部变量分配栈空间");
        }
    }
}

// 处理函数结束
void CodeGenerator::handleFunctionEnd(const Quadruple& q) {
    string proc_name = (q.arg1 == "main") ? "anchor_main" : q.arg1;
    emit("mov sp, bp", "释放局部变量空间");
    emit("pop bp", "恢复旧的基址指针");
    emit("ret", "返回");
    assembly_code << proc_name << " ENDP" << endl;
    current_function = "";
}

// 处理返回语句
void CodeGenerator::handleReturn(const Quadruple& q) {
    if (q.arg1 != "_") {
        emit("mov ax, " + getOperandAddress(q.arg1), "将返回值放入ax");
    }
    // 在返回前恢复栈帧
    emit("mov sp, bp");
    emit("pop bp");
    emit("ret");
}

// 处理参数压栈
void CodeGenerator::handleParam(const Quadruple& q) {
    emit("push " + getOperandAddress(q.arg1));
}

// 处理函数调用
void CodeGenerator::handleCall(const Quadruple& q) {
    string proc_name = (q.arg1 == "main") ? "anchor_main" : q.arg1;
    emit("call " + proc_name);
    if (!q.arg2.empty() && q.arg2 != "0") {
        int arg_count = stoi(q.arg2);
        if (arg_count > 0) {
            emit("add sp, " + to_string(arg_count * 2), "调用者清理参数占用的栈空间");
        }
    }
    if (q.res != "_") {
        emit("mov " + getOperandAddress(q.res) + ", ax", "保存返回值");
    }
}

// 处理打印语句
void CodeGenerator::handlePrint(const Quadruple& q) {
    string operand = q.arg1;
    auto type = getOperandType(operand);

    if (type && (type->name == "string" || type->name == "string[]" || (type->kind == TypeKind::ARRAY && type->elementType->name == "string"))) {
        emit("push " + getOperandAddress(operand), "压入字符串地址");
        emit("push OFFSET str_fmt", "压入字符串格式化符");
    } else { // 默认按 int/bool/float/char 处理
        emit("push " + getOperandAddress(operand), "压入整数值");
        emit("push OFFSET int_fmt", "压入整数格式化符");
    }
    emit("call _printf");
    emit("add sp, 4", "清理printf的参数栈");
}

// 处理比较操作
void CodeGenerator::handleComparison(const Quadruple& q) {
    string jump_instruction;
    if (q.op == "<") jump_instruction = "jl";
    else if (q.op == ">") jump_instruction = "jg";
    else if (q.op == "==") jump_instruction = "je";
    else if (q.op == "!=") jump_instruction = "jne";
    else if (q.op == ">=") jump_instruction = "jge";
    else if (q.op == "<=") jump_instruction = "jle";
    else return;

    string true_label = symbolTable.generateLabel();
    string end_label = symbolTable.generateLabel();

    emit("mov ax, " + getOperandAddress(q.arg1));
    emit("cmp ax, " + getOperandAddress(q.arg2));
    emit(jump_instruction + " " + true_label);
    emit("mov " + getOperandAddress(q.res) + ", 0", "结果为 false");
    emit("jmp " + end_label);
    assembly_code << true_label << ":" << endl;
    emit("mov " + getOperandAddress(q.res) + ", 1", "结果为 true");
    assembly_code << end_label << ":" << endl;
}

void CodeGenerator::handleArrayDeclaration(const Quadruple& q) {
    // 空间已在函数开始时通过 sub sp 统一分配，此处无需操作
}

// 处理向数组成员存值
void CodeGenerator::handleStoreAt(const Quadruple& q) {
    // 四元式: (STORE_AT, src, base, index)
    emit("mov bx, " + getOperandAddress(q.arg2), "将 index 放入 bx");
    emit("shl bx, 1", "index *= 2 (因为是WORD类型)");

    // 获取基地址到 si
    const Symbol* base_sym = symbolTable.lookup(q.res);
    if (base_sym && base_sym->type->kind == TypeKind::ARRAY && base_sym->scopeLevel > 0) {
        emit("lea si, " + getOperandAddress(q.res), "获取局部数组基地址");
    } else {
        emit("mov si, " + getOperandAddress(q.res), "获取全局或指针数组基地址");
    }

    emit("add si, bx", "计算最终地址");

    emit("mov ax, " + getOperandAddress(q.arg1), "将源值放入 ax");
    emit("mov [si], ax", "存入内存");
}

// 处理从数组成员取值
void CodeGenerator::handleLoadAt(const Quadruple& q) {
    // 四元式: (LOAD_AT, dest, base, index)
    emit("mov bx, " + getOperandAddress(q.res), "将 index 放入 bx");
    emit("shl bx, 1", "index *= 2");

    const Symbol* base_sym = symbolTable.lookup(q.arg2);
    // 修正：更准确地判断是取地址还是取值
    if (base_sym && base_sym->type->kind == TypeKind::ARRAY && base_sym->scopeLevel > 0) {
        emit("lea si, " + getOperandAddress(q.arg2), "获取局部数组基地址");
    } else {
        emit("mov si, " + getOperandAddress(q.arg2), "获取全局或指针数组基地址");
    }

    emit("add si, bx", "计算最终地址");
    emit("mov ax, [si]", "从内存取值");
    emit("mov " + getOperandAddress(q.arg1) + ", ax", "存入目标");
}

void CodeGenerator::handleGetParam(const Quadruple& q) {
    // 在当前模型中，参数直接通过 [bp+offset] 访问，此指令无需生成代码
}

// 发射单条汇编指令，附带可选注释
void CodeGenerator::emit(const std::string& instruction, const std::string& comment) {
    assembly_code << "    " << instruction;
    if (!comment.empty()) {
        assembly_code << " ; " << comment;
    }
    assembly_code << endl;
}
