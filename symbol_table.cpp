#include "symbol_table.h"
#include <sstream>

using namespace std;

// 构造函数
SymbolTable::SymbolTable() : currentOffset(0), tempVarCounter(0), labelCounter(0) {
    initializePrimitiveTypes();
    enterScope(); // 进入全局作用域
}

// 初始化基础类型并添加到类型系统中
void SymbolTable::initializePrimitiveTypes() {
    // name, size, alignment
    knownTypes["int"] = make_shared<TypeInfo>(TypeKind::PRIMITIVE, "int", 4, 4);
    knownTypes["float"] = make_shared<TypeInfo>(TypeKind::PRIMITIVE, "float", 8, 8);
    knownTypes["char"] = make_shared<TypeInfo>(TypeKind::PRIMITIVE, "char", 1, 1);
    knownTypes["bool"] = make_shared<TypeInfo>(TypeKind::PRIMITIVE, "bool", 1, 1);
    knownTypes["string"] = make_shared<TypeInfo>(TypeKind::PRIMITIVE, "string", 8, 8); // 假设字符串是指针大小
    knownTypes["void"] = make_shared<TypeInfo>(TypeKind::VOID_TYPE, "void", 0, 0);
}

void SymbolTable::enterScope() {
    scopes.push_back(unordered_map<string, Symbol>());
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

// 插入一个符号
// 参数按值传递，以允许我们设置 scopeLevel
bool SymbolTable::insert(Symbol symbol) {
    if (scopes.empty()) {
        cerr << "[致命错误] SymbolTable::insert 在没有活动作用域时被调用。" << endl;
        return false;
    }
    auto& currentScopeMap = scopes.back();

    if (currentScopeMap.count(symbol.name)) {
        cerr << "[语义错误] 标识符 '" << symbol.name
             << "' 在当前作用域中重复声明 (行 " << symbol.lineDeclared << ")" << endl;
        return false;
    }

    // 符号表自动设置作用域层级
    // 全局作用域是0, 第一个嵌套是1, 以此类推
    symbol.scopeLevel = scopes.size() - 1;

    currentScopeMap[symbol.name] = symbol;

    // 持久化存储用户定义的变量、函数和结构体类型
    if (symbol.category == SymbolCategory::Variable ||
        symbol.category == SymbolCategory::Function ||
        symbol.category == SymbolCategory::StructType) {
        allSymbolsEverDeclared[symbol.name] = symbol;
    }
    return true;
}

// 在作用域中查找符号
Symbol* SymbolTable::lookup(const string& name, bool currentScopeOnly) {
    if (currentScopeOnly) {
        if (!scopes.empty()) {
            auto& current = scopes.back();
            auto it = current.find(name);
            if (it != current.end()) {
                return &it->second;
            }
        }
    } else {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) {
                return &found->second;
            }
        }
    }
    return nullptr;
}

// 查找所有曾经声明过的符号
const Symbol* SymbolTable::lookupEverDeclared(const std::string& name) const {
    auto it = allSymbolsEverDeclared.find(name);
    if (it != allSymbolsEverDeclared.end()) {
        return &it->second;
    }
    return nullptr;
}

// 查找一个已知的类型
std::shared_ptr<TypeInfo> SymbolTable::lookupType(const std::string& typeName) {
    auto it = knownTypes.find(typeName);
    if (it != knownTypes.end()) {
        return it->second;
    }
    return nullptr; // 未找到
}

// 添加新的用户定义类型（如 struct）
void SymbolTable::addType(const std::string& typeName, std::shared_ptr<TypeInfo> typeInfo) {
    if (knownTypes.count(typeName)) {
        // 错误处理：类型重定义
        return;
    }
    knownTypes[typeName] = std::move(typeInfo);
}


void SymbolTable::dumpCurrentScope() const {
    cout << "=== 当前作用域符号表 ===" << endl;
    if (!scopes.empty()) {
        const auto& current = scopes.back();
        for (const auto& pair : current) {
            const Symbol& sym = pair.second;
            cout << "  " << sym.name << " (类型: " << (sym.type ? sym.type->name : "null")
                 << ", 类别: " << static_cast<int>(sym.category)
                 << ", 层级: " << sym.scopeLevel // **新增**: 打印层级
                 << ", 行号: " << sym.lineDeclared << ")" << endl;
        }
    } else {
        cout << "  (无活动作用域)" << endl;
    }
}

void SymbolTable::dumpAll() const {
    cout << "=== 所有作用域符号表 (编译时) ===" << endl;
    int level = 0;
    for (const auto& scopeMap : scopes) {
        cout << "[作用域 " << level++ << "]" << endl;
        if (scopeMap.empty()) {
            cout << "  (空)" << endl;
        }
        for (const auto& pair : scopeMap) {
            const Symbol& sym = pair.second;
             cout << "  " << sym.name << " (类型: " << (sym.type ? sym.type->name : "null")
                 << ", 类别: " << static_cast<int>(sym.category)
                 << ", 层级: " << sym.scopeLevel // 打印层级
                 << ", 行号: " << sym.lineDeclared << ")" << endl;
        }
    }
    cout << "--- 编译时作用域结束 ---" << endl;

    cout << "\n=== 所有曾声明的符号 (用于类型解析和解释器) ===" << endl;
    if (allSymbolsEverDeclared.empty()) {
        cout << "  (无)" << endl;
    }
    for (const auto& pair : allSymbolsEverDeclared) {
        const Symbol& sym = pair.second;
        cout << "  " << sym.name << " (类型: " << (sym.type ? sym.type->name : "null")
             << ", 类别: " << static_cast<int>(sym.category)
             << ", 层级: " << sym.scopeLevel // 新增打印层级
             << ", 行号: " << sym.lineDeclared << ")" << endl;
    }
    cout << "--- 所有曾声明的符号结束 ---" << endl;
}

std::string SymbolTable::generateTempVar() {
    return "T" + to_string(tempVarCounter++);
}

std::string SymbolTable::generateLabel() {
    return "L" + to_string(labelCounter++);
}

const std::unordered_map<std::string, Symbol>& SymbolTable::getAllSymbols() const {
    return allSymbolsEverDeclared;
}
