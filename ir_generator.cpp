#include "ir_generator.h"  // 包含IR生成器头文件
#include <iostream>         // 标准输入输出流
#include <algorithm>        // 标准算法库
#include <cctype>           // 字符处理函数
#include <vector>           // 向量容器

using namespace std;        // 使用标准命名空间

// IRGenerator构造函数
// 参数: root - AST根节点的唯一指针
//        st  - 符号表引用
IRGenerator::IRGenerator(unique_ptr<ASTNode> root, SymbolTable& st)
    : astRoot(std::move(root)),    // 转移AST根节点所有权
      symbolTable(st),             // 初始化符号表引用
      currentFunctionReturnType(nullptr) {} // 初始化当前函数返回类型为空

// 报告语义错误函数
// 参数: line    - 错误行号
//        message - 错误信息
void IRGenerator::reportSemanticError(int line, const string& message) {
    cerr << "语义错误 在行 " << line << ": " << message << endl; // 输出错误信息
    exit(EXIT_FAILURE);  // 终止程序
}

// IR生成入口函数
void IRGenerator::generate() {
    if (astRoot) {  // 检查AST根节点是否存在
        generate(astRoot.get());  // 从根节点开始生成
    }
}

// AST节点分发函数
// 根据节点类型调用对应的生成函数
void IRGenerator::generate(ASTNode* node) {
    if (!node) return;  // 空节点直接返回

    switch (node->nodeType) {
        case ASTNode::NodeType::Program:
            // 处理程序节点：生成语句列表
            generate(static_cast<ProgramNode*>(node)->statementList.get());
            break;

        case ASTNode::NodeType::StatementList:
            // 处理语句列表
            generateStatementList(static_cast<StatementListNode*>(node));
            break;

        case ASTNode::NodeType::DeclarationStatement:
            // 处理变量声明语句
            generateDeclarationStatement(static_cast<DeclarationStatementNode*>(node));
            break;

        case ASTNode::NodeType::AssignmentStatement:
            // 处理赋值语句
            generateAssignmentStatement(static_cast<AssignmentStatementNode*>(node));
            break;

        case ASTNode::NodeType::IfStatement:
            // 处理if语句
            generateIfStatement(static_cast<IfStatementNode*>(node));
            break;

        case ASTNode::NodeType::WhileStatement:
            // 处理while循环
            generateWhileStatement(static_cast<WhileStatementNode*>(node));
            break;

        case ASTNode::NodeType::PrintStatement:
            // 处理print语句
            generatePrintStatement(static_cast<PrintStatementNode*>(node));
            break;

        case ASTNode::NodeType::FunctionDefinition:
            // 处理函数定义
            generateFunctionDefinition(static_cast<FunctionDefinitionNode*>(node));
            break;

        case ASTNode::NodeType::ReturnStatement:
            // 处理return语句
            generateReturnStatement(static_cast<ReturnStatementNode*>(node));
            break;

        case ASTNode::NodeType::ForStatement:
            // 处理结构体定义
            generateForStatement(static_cast<ForStatementNode*>(node));
            break;

        case ASTNode::NodeType::StructiDefinitionStatement:
            // 处理结构体定义
            generateStructiDefinition(static_cast<StructiDefinitionNode*>(node));
            break;

        case ASTNode::NodeType::SwitchStatement:
            // 处理switch语句
            generateSwitchStatement(static_cast<SwitchStatementNode*>(node));
            break;

        case ASTNode::NodeType::BreakStatement:
            // 处理break语句
            generateBreakStatement(static_cast<BreakStatementNode*>(node));
            break;

        case ASTNode::NodeType::ContinueStatement:
            // 处理continue语句
            generateContinueStatement(static_cast<ContinueStatementNode*>(node));
            break;

        case ASTNode::NodeType::FunctionCall:
        case ASTNode::NodeType::BinaryExpression:
        case ASTNode::NodeType::UnaryExpression:
            // 处理表达式节点
            generateExpression(node);
            break;

        default:
            // 不支持节点类型报错
            reportSemanticError(node->lineNumber, "IRGenerator: 不支持此AST节点作为语句。");
    }
}

// 语句列表生成函数
void IRGenerator::generateStatementList(StatementListNode* node) {
    if (!node) return;  // 空节点直接返回

    // 遍历所有语句并生成代码
    for (const auto& stmt : node->statements) {
        if (stmt) generate(stmt.get());  // 递归生成每条语句
    }
}

// 从AST类型节点获取类型信息
std::shared_ptr<TypeInfo> IRGenerator::getTypeFromNode(ASTNode* typeNode) {
    if (!typeNode) return nullptr;  // 空节点返回空指针

    // 处理基础类型节点
    if (typeNode->nodeType == ASTNode::NodeType::Type) {
        auto baseTypeNode = static_cast<TypeNode*>(typeNode);
        return symbolTable.lookupType(baseTypeNode->typeName);  // 查找类型
    }

    // 处理数组类型节点
    if (typeNode->nodeType == ASTNode::NodeType::ArrayType) {
        auto arrayNode = static_cast<ArrayTypeNode*>(typeNode);
        auto elementType = getTypeFromNode(arrayNode->elementType.get());  // 获取元素类型

        // 元素类型检查
        if (!elementType) {
            reportSemanticError(arrayNode->lineNumber, "未知的数组元素类型。");
        }

        string typeName = elementType->name + "[]";  // 构造数组类型名
        // 创建数组类型信息 (假设指针/数组描述符大小为8字节)
        auto arrayType = make_shared<TypeInfo>(TypeKind::ARRAY, typeName, 8);
        arrayType->elementType = elementType;  // 设置元素类型
        arrayType->isDynamic = !arrayNode->sizeExpression;  // 判断是否为动态数组
        return arrayType;
    }

    // 无效类型节点报错
    reportSemanticError(typeNode->lineNumber, "无效的类型节点。");
    return nullptr;
}

// 变量声明语句处理函数
void IRGenerator::generateDeclarationStatement(DeclarationStatementNode* node) {
    // 获取变量类型
    auto varType = getTypeFromNode(node->typeSpecifier.get());
    if (!varType) {
        reportSemanticError(node->lineNumber, "变量 '" + node->identifierName + "' 的类型无效。");
    }

    // 创建变量符号
    Symbol varSymbol(node->identifierName, SymbolCategory::Variable, varType, node->lineNumber);
    // 插入符号表（检查重定义）
    if (!symbolTable.insert(varSymbol)) {
        reportSemanticError(node->lineNumber, "变量 '" + node->identifierName + "' 重定义。");
        return;
    }

    // 处理带初始化的声明
    if (node->initialValue) {
        // 初始化列表处理（数组初始化）
        if (node->initialValue->nodeType == ASTNode::NodeType::InitializerList) {
            // 验证只有数组类型允许初始化列表
            if (varType->kind != TypeKind::ARRAY) {
                reportSemanticError(node->lineNumber, "只有数组类型才能使用初始化列表进行初始化。");
            }
            // 递归初始化数组
            recursivelyInitializeArray(node->identifierName, varType,
                static_cast<InitializerListNode*>(node->initialValue.get()));
        }
        // 单值初始化处理
        else {
            // 生成初始化表达式
            ExpressionResult initRes = generateExpression(node->initialValue.get());
            // 检查类型兼容性
            if (!checkAssignmentCompatibility(varType, initRes.type, node->lineNumber)) {
                 reportSemanticError(node->lineNumber, "初始化值的类型 '" +
                    (initRes.type ? initRes.type->name : "null") +
                    "' 与变量类型 '" + varType->name + "' 不兼容。");
            }
            // 生成赋值四元式
            quadruples.push_back(Quadruple("=", initRes.place, "_", node->identifierName));
        }
    }
    // 无初始化的情况
    else {
        // 处理未初始化的静态数组
        if (varType->kind == TypeKind::ARRAY && !varType->isDynamic) {
             auto arrayNode = static_cast<ArrayTypeNode*>(node->typeSpecifier.get());
             // 静态数组必须指定大小
             if (!arrayNode->sizeExpression) {
                 reportSemanticError(node->lineNumber, "未初始化的静态数组声明必须指定大小。");
             }
            // 生成数组大小表达式
            auto sizeRes = generateExpression(arrayNode->sizeExpression.get());
            // 生成数组声明四元式
            quadruples.push_back(Quadruple("DEC_ARRAY", node->identifierName, sizeRes.place,
                to_string(varType->elementType->size)));
        }
        // 基础类型或动态数组无需额外操作
    }
}

// 递归初始化数组函数
std::string IRGenerator::recursivelyInitializeArray(const std::string& nameHint,
                                                    const std::shared_ptr<TypeInfo>& type,
                                                    InitializerListNode* initList) {
    // 检查类型是否为数组
    if (type->kind != TypeKind::ARRAY) {
        reportSemanticError(initList->lineNumber, "初始化列表只能用于数组类型。");
    }

    auto elementType = type->elementType;  // 获取元素类型
    int initSize = initList->elements.size();  // 获取初始化列表大小

    // 生成数组存储位置
    string arrayPlace = nameHint;
    if (nameHint.empty()) {
        arrayPlace = symbolTable.generateTempVar();  // 生成临时变量名
    }

    // 生成动态数组声明四元式
    quadruples.push_back(Quadruple("DEC_DYN_ARRAY", arrayPlace, to_string(initSize),
        to_string(elementType->size)));

    int index = 0;  // 初始化索引
    // 遍历初始化列表中的每个元素
    for (const auto& elemNode : initList->elements) {
        // 处理嵌套初始化列表（多维数组）
        if (elemNode->nodeType == ASTNode::NodeType::InitializerList) {
            // 检查元素类型是否为数组
            if (elementType->kind != TypeKind::ARRAY) {
                reportSemanticError(elemNode->lineNumber, "初始化列表的嵌套层级过多。");
            }
            // 递归初始化子数组
            string subArrayPlace = recursivelyInitializeArray("", elementType,
                static_cast<InitializerListNode*>(elemNode.get()));

            // 存储子数组指针
            quadruples.push_back(Quadruple("STORE_AT", subArrayPlace, arrayPlace, to_string(index)));

        }
        // 处理简单表达式元素
        else {
            // 检查元素类型
            if (elementType->kind == TypeKind::ARRAY) {
                reportSemanticError(elemNode->lineNumber, "初始化列表的嵌套层级不足，此处需要一个列表。");
            }
            // 生成元素表达式
            ExpressionResult elemRes = generateExpression(elemNode.get());
            // 检查类型兼容性
            if (!checkAssignmentCompatibility(elementType, elemRes.type, elemNode->lineNumber)) {
                 reportSemanticError(elemNode->lineNumber, "初始化列表中第 " +
                    to_string(index + 1) + " 个元素的类型与数组元素类型不兼容。");
            }
            // 存储元素值
            quadruples.push_back(Quadruple("STORE_AT", elemRes.place, arrayPlace, to_string(index)));
        }
        index++;  // 移动到下一个元素
    }

    return arrayPlace;  // 返回数组位置
}

// 函数定义处理函数
void IRGenerator::generateFunctionDefinition(FunctionDefinitionNode* node) {
    // 获取返回类型
    auto returnType = getTypeFromNode(node->returnType.get());
    if (!returnType) {
        reportSemanticError(node->lineNumber, "未知的函数返回类型。");
    }

    // 创建函数类型
    auto funcType = make_shared<TypeInfo>(TypeKind::FUNCTION, node->functionName, 0);
    funcType->returnType = returnType;  // 设置返回类型

    // 处理函数参数
    if (node->parameters) {
        for (const auto& paramNode : node->parameters->statements) {
            auto declNode = static_cast<DeclarationStatementNode*>(paramNode.get());
            auto paramType = getTypeFromNode(declNode->typeSpecifier.get());  // 获取参数类型
            if (!paramType) {
                reportSemanticError(paramNode->lineNumber, "未知的参数类型。");
            }
            // 添加参数信息
            funcType->parameters.push_back({declNode->identifierName, paramType});
        }
    }

    // 创建函数符号
    Symbol funcSymbol(node->functionName, SymbolCategory::Function, funcType, node->lineNumber);
    if (!symbolTable.insert(funcSymbol)) return;  // 插入符号表

    currentFunctionReturnType = returnType;  // 设置当前函数返回类型
    symbolTable.enterScope();  // 进入新作用域
    // 生成函数开始标签
    quadruples.push_back(Quadruple("FUNC_BEGIN", node->functionName, "_", "_"));

    // 处理函数参数
    for (const auto& param : funcType->parameters) {
        Symbol paramSymbol(param.name, SymbolCategory::Variable, param.type, node->lineNumber);
        symbolTable.insert(paramSymbol);  // 插入参数符号
        // 生成获取参数指令
        quadruples.push_back(Quadruple("GET_PARAM", param.name, "_", "_"));
    }

    generate(node->body.get());  // 生成函数体
    symbolTable.exitScope();  // 退出作用域
    // 生成函数结束标签
    quadruples.push_back(Quadruple("FUNC_END", node->functionName, "_", "_"));
    currentFunctionReturnType = nullptr;  // 重置当前函数返回类型
}

// 返回语句处理函数
void IRGenerator::generateReturnStatement(ReturnStatementNode* node) {
    // 检查是否在函数体内
    if (!currentFunctionReturnType) {
        reportSemanticError(node->lineNumber, "return 语句只能出现在函数体内。");
    }

    // 处理带返回值的return
    if (node->returnValue) {
        // 检查void函数是否有返回值
        if (currentFunctionReturnType->kind == TypeKind::VOID_TYPE) {
            reportSemanticError(node->lineNumber, "void 函数不能有返回值。");
        }
        // 生成返回值表达式
        ExpressionResult retRes = generateExpression(node->returnValue.get());
        // 检查返回类型兼容性
        if (!checkAssignmentCompatibility(currentFunctionReturnType, retRes.type, node->lineNumber)) {
            reportSemanticError(node->lineNumber, "返回值的类型 '" + retRes.type->name +
                "' 与函数声明的返回类型 '" + currentFunctionReturnType->name + "' 不匹配。");
        }
        // 生成返回指令
        quadruples.push_back(Quadruple("RETURN", retRes.place, "_", "_"));
    }
    // 处理无返回值的return
    else {
        // 检查非void函数是否缺少返回值
        if (currentFunctionReturnType->kind != TypeKind::VOID_TYPE) {
            reportSemanticError(node->lineNumber, "非 void 函数必须有返回值。");
        }
        // 生成空返回指令
        quadruples.push_back(Quadruple("RETURN", "_", "_", "_"));
    }
}

// 赋值语句处理函数
void IRGenerator::generateAssignmentStatement(AssignmentStatementNode* node) {
    // 生成右侧表达式
    ExpressionResult rhs = generateExpression(node->expression.get());

    string rhsPlace = rhs.place;  // 右侧结果位置

    // 处理复合赋值操作符 (如 +=, -= 等)
    if (node->op != "=") {
        string base_op = node->op;  // 获取基础操作符
        base_op.pop_back();  // 移除末尾的'='

        // 获取左侧原始值
        ExpressionResult lhs_original_value = generateExpression(node->leftHandSide.get(), false);
        string temp_result = symbolTable.generateTempVar();  // 生成临时变量

        // 生成复合赋值四元式
        quadruples.push_back(Quadruple(base_op, lhs_original_value.place, rhs.place, temp_result));
        rhsPlace = temp_result;  // 更新右侧结果位置
    }

    auto lhsNode = node->leftHandSide.get();  // 获取左侧节点

    // 处理不同类型的左侧表达式
    switch(lhsNode->nodeType) {
        case ASTNode::NodeType::Identifier: {
            auto idNode = static_cast<IdentifierNode*>(lhsNode);
            // 生成标识符表达式（需要左值）
            ExpressionResult lhs = generateIdentifier(idNode, true);
            // 检查是否为可修改的左值
            if (!lhs.isLValue) {
                reportSemanticError(node->lineNumber, "赋值号左边必须是可修改的左值。");
            }
            // 检查类型兼容性
            if (!checkAssignmentCompatibility(lhs.type, rhs.type, node->lineNumber)) {
                reportSemanticError(node->lineNumber, "赋值类型不兼容: 无法将 '" +
                    rhs.type->name + "' 赋给 '" + lhs.type->name + "'");
            }
            // 生成赋值指令
            quadruples.push_back(Quadruple("=", rhsPlace, "_", lhs.place));
            break;
        }

        case ASTNode::NodeType::ArrayAccessExpression: {
            auto arrayAccessNode = static_cast<ArrayAccessNode*>(lhsNode);
            // 生成数组表达式
            ExpressionResult arrayRes = generateExpression(arrayAccessNode->arrayExpr.get());
            // 生成索引表达式
            ExpressionResult indexRes = generateExpression(arrayAccessNode->indexExpr.get());

            // 检查是否为数组类型
            if (arrayRes.type->kind != TypeKind::ARRAY) {
                reportSemanticError(node->lineNumber, "无法对非数组类型进行下标赋值。");
            }
            // 检查元素类型兼容性
            if (!checkAssignmentCompatibility(arrayRes.type->elementType, rhs.type, node->lineNumber)) {
                reportSemanticError(node->lineNumber, "赋值类型不兼容: 无法将 '" +
                    rhs.type->name + "' 赋给 '" + arrayRes.type->elementType->name + "' 类型的数组成员");
            }

            // 生成数组元素存储指令
            quadruples.push_back(Quadruple("STORE_AT", rhsPlace, arrayRes.place, indexRes.place));
            break;
        }

        // 处理结构体成员访问
        case ASTNode::NodeType::MemberAccessExpression: {
            auto memberAccessNode = static_cast<MemberAccessNode*>(lhsNode);

            // 生成结构体表达式
            ExpressionResult baseRes = generateExpression(memberAccessNode->structExpr.get());

            // 检查是否为结构体类型
            if (!baseRes.isValid() || baseRes.type->kind != TypeKind::STRUCT) {
                reportSemanticError(node->lineNumber, "赋值号左侧的点运算符(.)只能用于结构体。");
            }

            // 查找成员偏移量和类型
            int memberOffset = -1;
            shared_ptr<TypeInfo> memberType = nullptr;
            for (const auto& member : baseRes.type->structMembers) {
                if (member.name == memberAccessNode->memberName) {
                    memberOffset = member.offset;
                    memberType = member.type;
                    break;
                }
            }

            // 检查成员是否存在
            if (memberOffset == -1) {
                reportSemanticError(node->lineNumber, "结构体 '" + baseRes.type->name +
                    "' 中没有名为 '" + memberAccessNode->memberName + "' 的成员。");
            }

            // 检查类型兼容性
            if (!checkAssignmentCompatibility(memberType, rhs.type, node->lineNumber)) {
                reportSemanticError(node->lineNumber, "赋值类型不兼容: 无法将 '" +
                    rhs.type->name + "' 赋给成员 '" + memberType->name + "'");
            }

            // 生成结构体成员存储指令
            quadruples.push_back(Quadruple("STORE_MEMBER", rhsPlace, baseRes.place, to_string(memberOffset)));
            break;
        }
        default:
            reportSemanticError(node->lineNumber, "赋值号左边必须是可修改的左值 (标识符或数组成员)。");
    }
}

// if语句处理函数
void IRGenerator::generateIfStatement(IfStatementNode* node) {
    // 生成条件表达式
    auto condRes = generateExpression(node->condition.get());
    // 检查条件是否为布尔类型
    if (!condRes.isValid() || condRes.type->name != "bool") {
        reportSemanticError(node->condition->lineNumber, "if 条件必须是布尔类型。");
    }

    // 生成标签
    string elseLabel = symbolTable.generateLabel();
    string endLabel = node->elseBlock ? symbolTable.generateLabel() : elseLabel;

    // 生成条件跳转指令
    quadruples.push_back(Quadruple("JUMPF", condRes.place, "_", elseLabel));
    // 生成then块代码
    generate(node->thenBlock.get());

    // 处理else块
    if (node->elseBlock) {
        // 生成跳转到结束标签的指令
        quadruples.push_back(Quadruple("JUMP", "_", "_", endLabel));
    }

    // 生成else标签
    quadruples.push_back(Quadruple("LABEL", elseLabel, "_", "_"));

    // 生成else块代码
    if (node->elseBlock) {
        generate(node->elseBlock.get());
        // 生成结束标签
        quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));
    }
}

// while循环处理函数
void IRGenerator::generateWhileStatement(WhileStatementNode* node) {
    // 生成标签
    string startLabel = symbolTable.generateLabel();
    string endLabel = symbolTable.generateLabel();

    // 设置break/continue上下文
    continueLabels.push_back(startLabel);
    breakLabels.push_back(endLabel);

    // 生成循环开始标签
    quadruples.push_back(Quadruple("LABEL", startLabel, "_", "_"));
    // 生成条件表达式
    auto condRes = generateExpression(node->condition.get());
    // 检查条件是否为布尔类型
    if(!condRes.isValid() || condRes.type->name != "bool")
        reportSemanticError(node->condition->lineNumber, "while 条件必须是布尔类型。");

    // 生成条件跳转指令
    quadruples.push_back(Quadruple("JUMPF", condRes.place, "_", endLabel));
    // 生成循环体代码
    generate(node->loopBlock.get());
    // 生成跳回循环开始的指令
    quadruples.push_back(Quadruple("JUMP", "_", "_", startLabel));
    // 生成循环结束标签
    quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));

    // 清除break/continue上下文
    continueLabels.pop_back();
    breakLabels.pop_back();
}

// print语句处理函数
void IRGenerator::generatePrintStatement(PrintStatementNode* node) {
    // 生成表达式
    auto exprRes = generateExpression(node->expression.get());
    // 检查表达式有效性
    if (!exprRes.isValid()) {
        reportSemanticError(node->lineNumber, "print 语句中的表达式无效。");
    }
    // 生成print指令
    quadruples.push_back(Quadruple("PRINT", exprRes.place, "_", "_"));
}

// 表达式生成函数
// 参数: needsLValue - 是否需要生成左值表达式
ExpressionResult IRGenerator::generateExpression(ASTNode* node, bool needsLValue) {
    if (!node) return ExpressionResult();  // 空节点返回空结果

    // 根据节点类型调用对应生成函数
    switch (node->nodeType) {
        case ASTNode::NodeType::Literal:
            return generateLiteral(static_cast<LiteralNode*>(node));  // 字面量
        case ASTNode::NodeType::Identifier:
            return generateIdentifier(static_cast<IdentifierNode*>(node), needsLValue);  // 标识符
        case ASTNode::NodeType::BinaryExpression:
            return generateBinaryExpression(static_cast<BinaryExpressionNode*>(node));  // 二元表达式
        case ASTNode::NodeType::UnaryExpression:
            return generateUnaryExpression(static_cast<UnaryExpressionNode*>(node));  // 一元表达式
        case ASTNode::NodeType::FunctionCall:
            return generateFunctionCall(static_cast<FunctionCallNode*>(node));  // 函数调用
        case ASTNode::NodeType::ArrayAccessExpression:
            return generateArrayAccess(static_cast<ArrayAccessNode*>(node), needsLValue);  // 数组访问
        case ASTNode::NodeType::MemberAccessExpression:
            return generateMemberAccess(static_cast<MemberAccessNode*>(node), needsLValue);  // 成员访问
        case ASTNode::NodeType::AssignmentStatement: {
            // 处理赋值表达式
            auto assignNode = static_cast<AssignmentStatementNode*>(node);
            generateAssignmentStatement(assignNode);  // 生成赋值语句
            return generateExpression(assignNode->leftHandSide.get(), false);  // 返回赋值结果
        }
        case ASTNode::NodeType::InitializerList:
            // 初始化列表不能作为独立表达式
            reportSemanticError(node->lineNumber, "初始化列表不能作为独立的表达式使用。");

        default:
            reportSemanticError(node->lineNumber, "不支持的表达式节点类型。");
    }
    return ExpressionResult();  // 默认返回空结果
}

// 函数调用处理函数
ExpressionResult IRGenerator::generateFunctionCall(FunctionCallNode* node) {
    // 特殊处理sizeof函数
    if (node->functionExpr->nodeType == ASTNode::NodeType::Identifier) {
        auto funcIdNode = static_cast<IdentifierNode*>(node->functionExpr.get());
        if (funcIdNode->name == "sizeof") {
            // 检查参数数量
            if (node->arguments.size() != 1) {
                reportSemanticError(node->lineNumber, "sizeof 函数需要且仅需要一个参数。");
            }
            // 获取参数类型
            auto type = getExpressionType(node->arguments[0].get());
            if (!type) {
                 reportSemanticError(node->arguments[0]->lineNumber, "无法确定 sizeof 参数的类型。");
            }
            // 返回类型大小
            return ExpressionResult(to_string(type->size), symbolTable.lookupType("int"), false);
        }
    }

    // 普通函数调用
    auto funcIdNode = static_cast<IdentifierNode*>(node->functionExpr.get());
    // 查找函数符号
    const Symbol* funcSymbol = symbolTable.lookup(funcIdNode->name);
    if (!funcSymbol || funcSymbol->category != SymbolCategory::Function) {
        reportSemanticError(node->lineNumber, "调用的标识符 '" + funcIdNode->name + "' 不是一个函数。");
    }
    auto funcType = funcSymbol->type;

    // 检查参数数量
    if (node->arguments.size() != funcType->parameters.size()) {
        reportSemanticError(node->lineNumber, "函数 '" + funcIdNode->name + "' 调用参数数量不匹配。");
    }

    // 从右向左处理参数（为了兼容参数入栈顺序）
    for (int i = node->arguments.size() - 1; i >= 0; --i) {
        // 生成参数表达式
        auto argRes = generateExpression(node->arguments[i].get());
        // 检查参数类型兼容性
        if (!checkAssignmentCompatibility(funcType->parameters[i].type, argRes.type, node->arguments[i]->lineNumber)) {
            reportSemanticError(node->arguments[i]->lineNumber, "函数调用中第 " + to_string(i+1) + " 个参数类型不匹配。");
        }
        // 生成参数传递指令
        quadruples.push_back(Quadruple("PARAM", argRes.place, "_", "_"));
    }

    // 生成函数调用指令
    string resultTemp = (funcType->returnType->kind != TypeKind::VOID_TYPE) ?
        symbolTable.generateTempVar() : "_";
    quadruples.push_back(Quadruple("CALL", funcIdNode->name, to_string(node->arguments.size()), resultTemp));

    // 返回函数调用结果
    return ExpressionResult(resultTemp, funcType->returnType, false);
}

// 数组访问处理函数
ExpressionResult IRGenerator::generateArrayAccess(ArrayAccessNode* node, bool needsLValue) {
    // 左值处理（用于赋值操作）
    if (needsLValue) {
        auto arrayRes = generateExpression(node->arrayExpr.get(), false);
        return ExpressionResult(arrayRes.place, arrayRes.type->elementType, true);
    }

    // 生成数组表达式
    ExpressionResult arrayRes = generateExpression(node->arrayExpr.get());
    // 生成索引表达式
    ExpressionResult indexRes = generateExpression(node->indexExpr.get());

    // 检查是否为数组类型
    if (!arrayRes.isValid() || arrayRes.type->kind != TypeKind::ARRAY) {
        reportSemanticError(node->lineNumber, "试图对非数组类型进行下标访问。");
    }
    // 检查索引是否为整数
    if (!indexRes.isValid() || indexRes.type->name != "int") {
        reportSemanticError(node->lineNumber, "数组索引必须是整数类型。");
    }

    // 获取元素类型
    auto elementType = arrayRes.type->elementType;
    string resultTemp = symbolTable.generateTempVar();  // 生成临时变量
    // 生成数组元素加载指令
    quadruples.push_back(Quadruple("LOAD_AT", resultTemp, arrayRes.place, indexRes.place));

    // 返回数组元素值
    return ExpressionResult(resultTemp, elementType, false);
}

// 二元表达式处理函数
ExpressionResult IRGenerator::generateBinaryExpression(BinaryExpressionNode* node) {
    // 生成左操作数
    auto lhs = generateExpression(node->left.get());
    // 生成右操作数
    auto rhs = generateExpression(node->right.get());
    // 检查操作类型兼容性
    auto resultType = checkOperationType(lhs.type, rhs.type, node->op, node->lineNumber);
    if (!resultType) {
        string lhs_name = lhs.type ? lhs.type->name : "无效类型";
        string rhs_name = rhs.type ? rhs.type->name : "无效类型";
        reportSemanticError(node->lineNumber, "二元操作符 '" + node->op +
            "' 的操作数类型不兼容 (" + lhs_name + ", " + rhs_name + ")");
    }

    // 生成临时变量存储结果
    string tempVar = symbolTable.generateTempVar();
    // 生成二元操作指令
    quadruples.push_back(Quadruple(node->op, lhs.place, rhs.place, tempVar));

    // 返回操作结果
    return ExpressionResult(tempVar, resultType, false);
}

// 一元表达式处理函数
ExpressionResult IRGenerator::generateUnaryExpression(UnaryExpressionNode* node) {
    // 生成操作数
    auto operandRes = generateExpression(node->operand.get());
    // 检查操作类型兼容性
    auto resultType = checkOperationType(operandRes.type, nullptr, node->op, node->lineNumber);
    if (!resultType) {
        string type_name = operandRes.type ? operandRes.type->name : "无效类型";
        reportSemanticError(node->lineNumber, "一元操作符 '" + node->op +
            "' 不支持类型 " + type_name);
    }

    // 生成临时变量存储结果
    string tempVar = symbolTable.generateTempVar();
    // 生成一元操作指令
    quadruples.push_back(Quadruple(node->op, operandRes.place, "_", tempVar));

    // 返回操作结果
    return ExpressionResult(tempVar, resultType, false);
}

// 字面量处理函数
ExpressionResult IRGenerator::generateLiteral(LiteralNode* node) {
    string typeName;  // 字面量类型名

    // 根据字面量类型确定类型名
    switch (node->literalType) {
        case TokenType::INT_LITERAL:    typeName = "int"; break;
        case TokenType::FLOAT_LITERAL:  typeName = "float"; break;
        case TokenType::STRING_LITERAL: typeName = "string"; break;
        case TokenType::CHAR_LITERAL:   typeName = "char"; break;
        case TokenType::KW_TRUE:
        case TokenType::KW_FALSE:       typeName = "bool"; break;
        default:
            reportSemanticError(node->lineNumber, "未知的字面量类型。");
    }

    // 查找类型信息
    auto typeInfo = symbolTable.lookupType(typeName);
    if (!typeInfo) {
        reportSemanticError(node->lineNumber, "内部错误：在符号表中找不到基础类型 '" + typeName + "'。");
    }

    // 返回字面量表达式结果
    return ExpressionResult(node->value, typeInfo, false);
}

// 标识符处理函数
ExpressionResult IRGenerator::generateIdentifier(IdentifierNode* node, bool needsLValue) {
    // 查找标识符符号
    const Symbol* sym = symbolTable.lookup(node->name);
    if(!sym) {
        reportSemanticError(node->lineNumber, "未声明的标识符: " + node->name);
    }

    // 检查函数名误用
    if (sym->category == SymbolCategory::Function) {
        reportSemanticError(node->lineNumber, "函数名 '" + node->name + "' 只能用于函数调用。");
    }

    // 确定是否为左值
    bool isLVal = (sym->category == SymbolCategory::Variable);
    // 返回标识符表达式结果
    return ExpressionResult(node->name, sym->type, isLVal);
}

// 成员访问处理函数
ExpressionResult IRGenerator::generateMemberAccess(MemberAccessNode* node, bool needsLValue) {
    // 生成结构体表达式
    ExpressionResult baseRes = generateExpression(node->structExpr.get());

    // 检查是否为结构体类型
    if (!baseRes.isValid() || baseRes.type->kind != TypeKind::STRUCT) {
        reportSemanticError(node->lineNumber, "点运算符(.)只能用于结构体类型。");
    }

    // 查找成员信息
    int memberOffset = -1;
    shared_ptr<TypeInfo> memberType = nullptr;
    for (const auto& member : baseRes.type->structMembers) {
        if (member.name == node->memberName) {
            memberOffset = member.offset;
            memberType = member.type;
            break;
        }
    }

    // 检查成员是否存在
    if (memberOffset == -1) {
        reportSemanticError(node->lineNumber, "结构体 '" + baseRes.type->name +
            "' 中没有名为 '" + node->memberName + "' 的成员。");
    }

    // 生成临时变量存储结果
    string resultTemp = symbolTable.generateTempVar();
    // 生成成员加载指令
    quadruples.push_back(Quadruple("LOAD_MEMBER", resultTemp, baseRes.place, to_string(memberOffset)));

    // 返回成员值
    return ExpressionResult(resultTemp, memberType, false);
}

// 获取表达式类型函数
std::shared_ptr<TypeInfo> IRGenerator::getExpressionType(ASTNode* node) {
    if (!node) return nullptr;  // 空节点返回空

    // 标识符节点：从符号表获取类型
    if (node->nodeType == ASTNode::NodeType::Identifier) {
        auto idNode = static_cast<IdentifierNode*>(node);
        const Symbol* symbol = symbolTable.lookup(idNode->name);
        if (symbol) return symbol->type;
    }
    return nullptr;  // 默认返回空
}

// 检查赋值兼容性函数
bool IRGenerator::checkAssignmentCompatibility(const std::shared_ptr<TypeInfo>& target,
                                             const std::shared_ptr<TypeInfo>& source, int line) {
    // 空类型检查
    if (!target || !source ||
        target->kind == TypeKind::UNKNOWN ||
        source->kind == TypeKind::UNKNOWN) return false;

    // void类型不兼容
    if (target->kind == TypeKind::VOID_TYPE ||
        source->kind == TypeKind::VOID_TYPE) return false;

    // 相同类型兼容
    if (target->name == source->name) return true;

    // int到float的隐式转换
    if (target->name == "float" && source->name == "int") return true;

    return false;  // 默认不兼容
}

// 检查操作类型兼容性函数
std::shared_ptr<TypeInfo> IRGenerator::checkOperationType(const std::shared_ptr<TypeInfo>& type1,
                                                         const std::shared_ptr<TypeInfo>& type2,
                                                         const std::string& op, int line) {
    if (!type1) return nullptr;  // 空类型返回空

    // 二元运算符处理
    if (type2) {
        // 字符串拼接
        if (op == "+") {
            if ((type1->name == "string" && type2->kind == TypeKind::PRIMITIVE) ||
                (type2->name == "string" && type1->kind == TypeKind::PRIMITIVE)) {
                return symbolTable.lookupType("string");  // 返回字符串类型
            }
        }

        // 数值运算
        if ((op == "+" || op == "-" || op == "*" || op == "/") &&
            (type1->name == "int" || type1->name == "float") &&
            (type2->name == "int" || type2->name == "float")) {
            // 浮点优先
            if (type1->name == "float" || type2->name == "float")
                return symbolTable.lookupType("float");
            return symbolTable.lookupType("int");  // 否则返回int
        }

        // 比较运算
        if (op == ">" || op == "<" || op == ">=" || op == "<=" || op == "==" || op == "!=") {
            if (type1->name == type2->name && type1->kind == TypeKind::PRIMITIVE) {
                return symbolTable.lookupType("bool");  // 返回布尔类型
            }
        }

        // 逻辑运算
        if ((op == "&&" || op == "||") &&
            type1->name == "bool" && type2->name == "bool") {
            return symbolTable.lookupType("bool");  // 返回布尔类型
        }
    }
    // 一元运算符处理
    else {
        if(op == "!") {
            if (type1->name == "bool") return symbolTable.lookupType("bool");  // 逻辑非
        }
        if(op == "-") {
            if (type1->name == "int" || type1->name == "float") return type1;  // 数值取负
        }
    }

    return nullptr;  // 不兼容操作
}

// for循环处理函数
void IRGenerator::generateForStatement(ForStatementNode* node) {
    // 进入新作用域（支持循环内变量声明）
    symbolTable.enterScope();

    // 生成初始化代码
    if (node->initialization) {
        generate(node->initialization.get());
    }

    // 生成循环标签
    string conditionLabel = symbolTable.generateLabel(); // 条件判断入口
    string incrementLabel = symbolTable.generateLabel(); // continue跳转目标
    string endLabel = symbolTable.generateLabel();      // break跳转目标

    // 设置break/continue上下文
    breakLabels.push_back(endLabel);
    continueLabels.push_back(incrementLabel);

    // 生成条件判断标签
    quadruples.push_back(Quadruple("LABEL", conditionLabel, "_", "_"));

    // 处理条件表达式
    if (node->condition) {
        // 生成条件表达式
        ExpressionResult condRes = generateExpression(node->condition.get());
        // 检查条件是否为布尔类型
        if (!condRes.isValid() || condRes.type->name != "bool") {
            reportSemanticError(node->condition->lineNumber, "for 循环的条件必须是布尔类型。");
        }
        // 生成条件跳转指令
        quadruples.push_back(Quadruple("JUMPF", condRes.place, "_", endLabel));
    }

    // 生成循环体代码
    if (node->body) {
        generate(node->body.get());
    }

    // 生成增量语句标签
    quadruples.push_back(Quadruple("LABEL", incrementLabel, "_", "_"));

    // 生成增量语句
    if (node->increment) {
        generateExpression(node->increment.get());
    }

    // 生成跳回条件判断的指令
    quadruples.push_back(Quadruple("JUMP", "_", "_", conditionLabel));

    // 生成循环结束标签
    quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));

    // 清除break/continue上下文
    continueLabels.pop_back();
    breakLabels.pop_back();

    // 退出作用域
    symbolTable.exitScope();
}

// 结构体定义处理函数
void IRGenerator::generateStructiDefinition(StructiDefinitionNode* node) {
    // 检查结构体是否已定义
    if (symbolTable.lookupType(node->structiName) != nullptr) {
        reportSemanticError(node->lineNumber, "结构体 '" + node->structiName + "' 重复定义。");
        return;
    }

    // 创建结构体类型
    auto structType = make_shared<TypeInfo>(TypeKind::STRUCT, node->structiName, 0);
    int currentOffset = 0;  // 当前成员偏移量
    int totalSize = 0;     // 结构体总大小

    // 处理成员声明
    for (const auto& memberNode : node->memberDeclarations->statements) {
        auto declNode = static_cast<DeclarationStatementNode*>(memberNode.get());
        // 获取成员类型
        auto memberType = getTypeFromNode(declNode->typeSpecifier.get());
        if (!memberType) {
            reportSemanticError(declNode->lineNumber, "结构体成员 '" + declNode->identifierName + "' 的类型未知。");
            continue;
        }

        // 添加成员信息
        structType->structMembers.push_back({declNode->identifierName, memberType, currentOffset});

        // 更新偏移量和总大小（简化实现，假设所有成员占2字节）
        int memberSize = 2;
        currentOffset += memberSize;
        totalSize += memberSize;
    }

    structType->size = totalSize; // 设置结构体大小

    // 添加结构体类型到符号表
    symbolTable.addType(node->structiName, structType);
}

// switch语句处理函数
void IRGenerator::generateSwitchStatement(SwitchStatementNode* node) {
    // 生成switch表达式
    ExpressionResult switchExpr = generateExpression(node->expression.get());
    // 检查表达式类型
    if (switchExpr.type->name != "int" && switchExpr.type->name != "char") {
        reportSemanticError(node->lineNumber, "switch 语句的表达式必须是整数或字符类型。");
    }

    // 生成标签
    string endLabel = symbolTable.generateLabel();
    string defaultLabel = "";
    vector<pair<string, string>> caseLabels;

    // 处理case语句
    for (const auto& caseNode : node->cases) {
        if (caseNode->value) {
            // 生成case标签
            string caseBodyLabel = symbolTable.generateLabel();
            caseLabels.push_back({caseBodyLabel, ""});

            // 生成case值表达式
            ExpressionResult caseValue = generateExpression(caseNode->value.get());
            // 检查类型兼容性
            if (!checkAssignmentCompatibility(switchExpr.type, caseValue.type, caseNode->lineNumber)) {
                reportSemanticError(caseNode->lineNumber, "case 标签的类型与 switch 表达式的类型不匹配。");
            }

            // 生成比较表达式
            string tempVar = symbolTable.generateTempVar();
            quadruples.push_back(Quadruple("==", switchExpr.place, caseValue.place, tempVar));
            // 生成条件跳转
            quadruples.push_back(Quadruple("JUMPNZ", tempVar, "_", caseBodyLabel));
        } else {
            // 处理default标签
            if (!defaultLabel.empty()) {
                reportSemanticError(caseNode->lineNumber, "一个 switch 语句中只能有一个 default 标签。");
            }
            defaultLabel = symbolTable.generateLabel();
        }
    }

    // 生成默认跳转
    if (!defaultLabel.empty()) {
        quadruples.push_back(Quadruple("JUMP", "_", "_", defaultLabel));
    } else {
        quadruples.push_back(Quadruple("JUMP", "_", "_", endLabel));
    }

    // 设置break上下文
    breakLabels.push_back(endLabel);
    int caseIndex = 0;

    // 生成case代码
    for (const auto& caseNode : node->cases) {
        if (caseNode->value) {
            // 生成case标签
            quadruples.push_back(Quadruple("LABEL", caseLabels[caseIndex].first, "_", "_"));
            // 生成case代码体
            generateStatementList(caseNode->body.get());
            caseIndex++;
        } else {
            // 生成default标签
            quadruples.push_back(Quadruple("LABEL", defaultLabel, "_", "_"));
            // 生成default代码体
            generateStatementList(caseNode->body.get());
        }
    }
    // 清除break上下文
    breakLabels.pop_back();

    // 生成结束标签
    quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));
}

// break语句处理函数
void IRGenerator::generateBreakStatement(BreakStatementNode* node) {
    // 检查break上下文
    if (breakLabels.empty()) {
        reportSemanticError(node->lineNumber, "break 语句只能出现在循环或switch语句中。");
    }
    // 生成跳转到break标签的指令
    quadruples.push_back(Quadruple("JUMP", "_", "_", breakLabels.back()));
}

// continue语句处理函数
void IRGenerator::generateContinueStatement(ContinueStatementNode* node) {
    // 检查continue上下文
    if (continueLabels.empty()) {
        reportSemanticError(node->lineNumber, "continue 语句只能出现在循环语句中。");
    }
    // 生成跳转到continue标签的指令
    quadruples.push_back(Quadruple("JUMP", "_", "_", continueLabels.back()));
}

// 输出四元式函数（调试用）
void IRGenerator::dumpQuadruples() const {
    cout << "\n--- 生成的四元式 ---" << endl;
    // 遍历并输出所有四元式
    for (size_t i = 0; i < quadruples.size(); ++i) {
        cout << i << ":\t" << quadruples[i].toString() << endl;
    }
    cout << "--- 四元式结束 ---" << endl;
}
