// quadruple.h
#ifndef QUADRUPLE_H
#define QUADRUPLE_H

#include <string>

// 四元式结构体：(Operator, Operand1, Operand2, Result)
struct Quadruple {
    std::string op;    // 操作符 (例如 "+", "=", "JUMPF", "PRINT")
    std::string arg1;  // 操作数1
    std::string arg2;  // 操作数2
    std::string res;   // 结果或目标标签

    Quadruple(const std::string& oper, const std::string& a1, const std::string& a2, const std::string& r)
        : op(oper), arg1(a1), arg2(a2), res(r) {}

    // 用于调试打印四元式
    std::string toString() const {
        return "(" + op + ", " + arg1 + ", " + arg2 + ", " + res + ")";
    }
};

#endif // QUADRUPLE_H
