#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <utility>

//类型系统
// 前向声明，以支持指针和递归类型定义
struct TypeInfo;

// 用于函数参数的结构体
struct ParameterInfo {
    std::string name;
    std::shared_ptr<TypeInfo> type;
};

//用于描述结构题的成员的结构体（我的附庸的附庸不是我的附庸，bushi）
struct StructMemberInfo {
    std::string name;
    std::shared_ptr<TypeInfo> type;
    int offset;
};

// 类型的种类枚举
enum class TypeKind {
    PRIMITIVE, // 如 int, float, bool
    STRUCT,    // 结构体
    ARRAY,     // 数组
    FUNCTION,  // 函数
    VOID_TYPE, // 特殊的 void 类型
    UNKNOWN    // 未知或错误类型
};

// 结构化的类型信息
struct TypeInfo {
    TypeKind kind = TypeKind::UNKNOWN;
    std::string name; // 基础类型名("int")、结构体名或函数名
    int size = 0;     // 类型占用的内存大小
    int alignment = 0;// 内存对齐要求

    // --- 仅当 kind == ARRAY 时有效 ---
    std::shared_ptr<TypeInfo> elementType = nullptr; // 数组的元素类型
    int arrayElementCount = 0;                       // 【静态数组】的元素数量
    bool isDynamic = false;                          // 标记是否为动态数组

    // 仅当 kind == FUNCTION 时有效
    std::shared_ptr<TypeInfo> returnType = nullptr;       // 函数返回值类型
    std::vector<ParameterInfo> parameters;              // 函数参数列表

    // 仅当 kind == STRUCT 时有xiao
    std::vector<StructMemberInfo> structMembers;
    // 默认构造
    TypeInfo() = default;

    // 构造函数
    TypeInfo(TypeKind k, std::string n, int s, int a = 1)
        : kind(k), name(std::move(n)), size(s), alignment(a) {}
};

// 符号的类别
enum class SymbolCategory {
    Keyword,
    Variable,       // 变量实例
    Constant,
    StructType,     // 结构体类型的定义
    Function        // 函数的定义
};

// 符号表中的条目
struct Symbol {
    std::string name;
    SymbolCategory category;
    std::shared_ptr<TypeInfo> type; // 使用 TypeInfo 代替 std::string
    bool isConst;
    bool isInitialized;
    int memoryOffset;
    int lineDeclared;
    int scopeLevel;

    Symbol() : isConst(false), isInitialized(false), memoryOffset(-1), lineDeclared(-1), scopeLevel(-1) {}

    Symbol(std::string n, SymbolCategory cat, std::shared_ptr<TypeInfo> t,
           int line, bool cst = false, bool init = false, int offset = -1)
        : name(std::move(n)), category(cat), type(std::move(t)), isConst(cst),
          isInitialized(init), memoryOffset(offset), lineDeclared(line), scopeLevel(-1) {}
};

// 符号表类
class SymbolTable {
private:
    std::vector<std::unordered_map<std::string, Symbol>> scopes;
    int currentOffset = 0;
    int tempVarCounter = 0;
    int labelCounter = 0;

    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> knownTypes;

    std::unordered_map<std::string, Symbol> allSymbolsEverDeclared;

    void initializePrimitiveTypes();

public:
    SymbolTable();

    void enterScope();
    void exitScope();

    bool insert(Symbol symbol);

    Symbol* lookup(const std::string& name, bool currentScopeOnly = false);
    const Symbol* lookupEverDeclared(const std::string& name) const;

    std::shared_ptr<TypeInfo> lookupType(const std::string& typeName);
    void addType(const std::string& typeName, std::shared_ptr<TypeInfo> typeInfo);

    void dumpCurrentScope() const;
    void dumpAll() const;

    std::string generateTempVar();
    std::string generateLabel();

    const std::unordered_map<std::string, Symbol> &getAllSymbols() const;
};

#endif // SYMBOL_TABLE_H
