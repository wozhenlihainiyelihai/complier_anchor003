#include "ir_generator.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <vector>

using namespace std;

IRGenerator::IRGenerator(unique_ptr<ASTNode> root, SymbolTable& st)
    : astRoot(std::move(root)), symbolTable(st), currentFunctionReturnType(nullptr) {}

void IRGenerator::reportSemanticError(int line, const string& message) {
    cerr << "语义错误 在行 " << line << ": " << message << endl;
    exit(EXIT_FAILURE);
}

void IRGenerator::generate() {
    if (astRoot) {
        generate(astRoot.get());
    }
}

void IRGenerator::generate(ASTNode* node) {
    if (!node) return;
    switch (node->nodeType) {
        case ASTNode::NodeType::Program:
            generate(static_cast<ProgramNode*>(node)->statementList.get());
            break;
        case ASTNode::NodeType::StatementList:
            generateStatementList(static_cast<StatementListNode*>(node));
            break;
        case ASTNode::NodeType::DeclarationStatement:
            generateDeclarationStatement(static_cast<DeclarationStatementNode*>(node));
            break;
        case ASTNode::NodeType::AssignmentStatement:
            generateAssignmentStatement(static_cast<AssignmentStatementNode*>(node));
            break;
        case ASTNode::NodeType::IfStatement:
            generateIfStatement(static_cast<IfStatementNode*>(node));
            break;
        case ASTNode::NodeType::WhileStatement:
            generateWhileStatement(static_cast<WhileStatementNode*>(node));
            break;
        case ASTNode::NodeType::PrintStatement:
            generatePrintStatement(static_cast<PrintStatementNode*>(node));
            break;
        case ASTNode::NodeType::FunctionDefinition:
            generateFunctionDefinition(static_cast<FunctionDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::ReturnStatement:
            generateReturnStatement(static_cast<ReturnStatementNode*>(node));
            break;
        case ASTNode::NodeType::ForStatement:
             generateForStatement(static_cast<ForStatementNode*>(node));
            break;
        case ASTNode::NodeType::StructiDefinitionStatement:
            generateStructiDefinition(static_cast<StructiDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::SwitchStatement:
            generateSwitchStatement(static_cast<SwitchStatementNode*>(node));
            break;
        case ASTNode::NodeType::BreakStatement:
            generateBreakStatement(static_cast<BreakStatementNode*>(node));
            break;
        case ASTNode::NodeType::ContinueStatement:
            generateContinueStatement(static_cast<ContinueStatementNode*>(node));
            break;
        case ASTNode::NodeType::FunctionCall:
        case ASTNode::NodeType::BinaryExpression:
        case ASTNode::NodeType::UnaryExpression:
            generateExpression(node);
            break;
        // 修正: 从分发器中移除InitializerList，因为它不应作为独立语句存在
        // case ASTNode::NodeType::InitializerList:
        //     generateExpression(node);
        //     break;
        default:
            reportSemanticError(node->lineNumber, "IRGenerator: 不支持此AST节点作为语句。");
    }
}

void IRGenerator::generateStatementList(StatementListNode* node) {
    if (!node) return;
    for (const auto& stmt : node->statements) {
        if (stmt) generate(stmt.get());
    }
}

std::shared_ptr<TypeInfo> IRGenerator::getTypeFromNode(ASTNode* typeNode) {
    if (!typeNode) return nullptr;

    if (typeNode->nodeType == ASTNode::NodeType::Type) {
        auto baseTypeNode = static_cast<TypeNode*>(typeNode);
        return symbolTable.lookupType(baseTypeNode->typeName);
    }

    if (typeNode->nodeType == ASTNode::NodeType::ArrayType) {
        auto arrayNode = static_cast<ArrayTypeNode*>(typeNode);
        auto elementType = getTypeFromNode(arrayNode->elementType.get());
        if (!elementType) {
            reportSemanticError(arrayNode->lineNumber, "未知的数组元素类型。");
        }

        string typeName = elementType->name + "[]";
        // 假设指针/数组描述符的大小为8字节
        auto arrayType = make_shared<TypeInfo>(TypeKind::ARRAY, typeName, 8);
        arrayType->elementType = elementType;
        arrayType->isDynamic = !arrayNode->sizeExpression;
        return arrayType;
    }

    reportSemanticError(typeNode->lineNumber, "无效的类型节点。");
    return nullptr;
}

void IRGenerator::generateDeclarationStatement(DeclarationStatementNode* node) {
    auto varType = getTypeFromNode(node->typeSpecifier.get());
    if (!varType) {
        reportSemanticError(node->lineNumber, "变量 '" + node->identifierName + "' 的类型无效。");
    }

    Symbol varSymbol(node->identifierName, SymbolCategory::Variable, varType, node->lineNumber);
    if (!symbolTable.insert(varSymbol)) {
        reportSemanticError(node->lineNumber, "变量 '" + node->identifierName + "' 重定义。");
        return;
    }

    if (node->initialValue) {
        if (node->initialValue->nodeType == ASTNode::NodeType::InitializerList) {
            // --- 使用新的递归函数处理初始化列表 ---
            if (varType->kind != TypeKind::ARRAY) {
                reportSemanticError(node->lineNumber, "只有数组类型才能使用初始化列表进行初始化。");
            }
            recursivelyInitializeArray(node->identifierName, varType, static_cast<InitializerListNode*>(node->initialValue.get()));
        } else {
            // --- 单个表达式初始化 ---
            ExpressionResult initRes = generateExpression(node->initialValue.get());
            if (!checkAssignmentCompatibility(varType, initRes.type, node->lineNumber)) {
                 reportSemanticError(node->lineNumber, "初始化值的类型 '" + (initRes.type ? initRes.type->name : "null") + "' 与变量类型 '" + varType->name + "' 不兼容。");
            }
            quadruples.push_back(Quadruple("=", initRes.place, "_", node->identifierName));
        }
    } else {
        // --- 无初始化 ---
        if (varType->kind == TypeKind::ARRAY && !varType->isDynamic) {
             auto arrayNode = static_cast<ArrayTypeNode*>(node->typeSpecifier.get());
             if (!arrayNode->sizeExpression) {
                 reportSemanticError(node->lineNumber, "未初始化的静态数组声明必须指定大小。");
             }
            auto sizeRes = generateExpression(arrayNode->sizeExpression.get());
            quadruples.push_back(Quadruple("DEC_ARRAY", node->identifierName, sizeRes.place, to_string(varType->elementType->size)));
        }
        // 对于简单类型 (int x;) 或动态数组 (int[] a;)，在运行时分配前无需生成IR
    }
}

std::string IRGenerator::recursivelyInitializeArray(const std::string& nameHint,
                                                    const std::shared_ptr<TypeInfo>& type,
                                                    InitializerListNode* initList) {
    if (type->kind != TypeKind::ARRAY) {
        reportSemanticError(initList->lineNumber, "初始化列表只能用于数组类型。");
    }

    auto elementType = type->elementType;
    int initSize = initList->elements.size();

    // 如果没有提供名称（用于子数组），则生成一个临时变量名
    string arrayPlace = nameHint;
    if (nameHint.empty()) {
        arrayPlace = symbolTable.generateTempVar();
    }

    // 声明当前维度的数组。我们总是使用动态数组指令，因为初始化列表决定了大小。
    // `elementType->size` 对于子数组来说，应该是指针的大小。
    quadruples.push_back(Quadruple("DEC_DYN_ARRAY", arrayPlace, to_string(initSize), to_string(elementType->size)));

    int index = 0;
    for (const auto& elemNode : initList->elements) {
        if (elemNode->nodeType == ASTNode::NodeType::InitializerList) {
            // --- 元素是另一个列表（即子数组）---
            if (elementType->kind != TypeKind::ARRAY) {
                reportSemanticError(elemNode->lineNumber, "初始化列表的嵌套层级过多。");
            }
            // 递归调用以初始化子数组。它将返回子数组的临时变量名（例如 "T1"）。
            string subArrayPlace = recursivelyInitializeArray("", elementType, static_cast<InitializerListNode*>(elemNode.get()));

            // 将指向子数组的指针存入当前数组
            quadruples.push_back(Quadruple("STORE_AT", subArrayPlace, arrayPlace, to_string(index)));

        } else {
            // --- 元素是简单表达式 ---
            if (elementType->kind == TypeKind::ARRAY) {
                reportSemanticError(elemNode->lineNumber, "初始化列表的嵌套层级不足，此处需要一个列表。");
            }
            ExpressionResult elemRes = generateExpression(elemNode.get());
            if (!checkAssignmentCompatibility(elementType, elemRes.type, elemNode->lineNumber)) {
                 reportSemanticError(elemNode->lineNumber, "初始化列表中第 " + to_string(index + 1) + " 个元素的类型与数组元素类型不兼容。");
            }
            quadruples.push_back(Quadruple("STORE_AT", elemRes.place, arrayPlace, to_string(index)));
        }
        index++;
    }

    return arrayPlace;
}

void IRGenerator::generateFunctionDefinition(FunctionDefinitionNode* node) {
    auto returnType = getTypeFromNode(node->returnType.get());
    if (!returnType) {
        reportSemanticError(node->lineNumber, "未知的函数返回类型。");
    }
    auto funcType = make_shared<TypeInfo>(TypeKind::FUNCTION, node->functionName, 0);
    funcType->returnType = returnType;
    if (node->parameters) {
        for (const auto& paramNode : node->parameters->statements) {
            auto declNode = static_cast<DeclarationStatementNode*>(paramNode.get());
            auto paramType = getTypeFromNode(declNode->typeSpecifier.get());
            if (!paramType) {
                reportSemanticError(paramNode->lineNumber, "未知的参数类型。");
            }
            funcType->parameters.push_back({declNode->identifierName, paramType});
        }
    }
    Symbol funcSymbol(node->functionName, SymbolCategory::Function, funcType, node->lineNumber);
    if (!symbolTable.insert(funcSymbol)) return;

    currentFunctionReturnType = returnType;
    symbolTable.enterScope();
    quadruples.push_back(Quadruple("FUNC_BEGIN", node->functionName, "_", "_"));
    for (const auto& param : funcType->parameters) {
        Symbol paramSymbol(param.name, SymbolCategory::Variable, param.type, node->lineNumber);
        symbolTable.insert(paramSymbol);
        quadruples.push_back(Quadruple("GET_PARAM", param.name, "_", "_"));
    }
    generate(node->body.get());
    symbolTable.exitScope();
    quadruples.push_back(Quadruple("FUNC_END", node->functionName, "_", "_"));
    currentFunctionReturnType = nullptr;
}

void IRGenerator::generateReturnStatement(ReturnStatementNode* node) {
    if (!currentFunctionReturnType) {
        reportSemanticError(node->lineNumber, "return 语句只能出现在函数体内。");
    }
    if (node->returnValue) {
        if (currentFunctionReturnType->kind == TypeKind::VOID_TYPE) {
            reportSemanticError(node->lineNumber, "void 函数不能有返回值。");
        }
        ExpressionResult retRes = generateExpression(node->returnValue.get());
        if (!checkAssignmentCompatibility(currentFunctionReturnType, retRes.type, node->lineNumber)) {
            reportSemanticError(node->lineNumber, "返回值的类型 '" + retRes.type->name + "' 与函数声明的返回类型 '" + currentFunctionReturnType->name + "' 不匹配。");
        }
        quadruples.push_back(Quadruple("RETURN", retRes.place, "_", "_"));
    } else {
        if (currentFunctionReturnType->kind != TypeKind::VOID_TYPE) {
            reportSemanticError(node->lineNumber, "非 void 函数必须有返回值。");
        }
        quadruples.push_back(Quadruple("RETURN", "_", "_", "_"));
    }
}

void IRGenerator::generateAssignmentStatement(AssignmentStatementNode* node) {
    ExpressionResult rhs = generateExpression(node->expression.get());

    string rhsPlace = rhs.place;
    if (node->op != "=") {
        string base_op = node->op;
        base_op.pop_back();

        ExpressionResult lhs_original_value = generateExpression(node->leftHandSide.get(), false);
        string temp_result = symbolTable.generateTempVar();
        quadruples.push_back(Quadruple(base_op, lhs_original_value.place, rhs.place, temp_result));
        rhsPlace = temp_result;
    }

    auto lhsNode = node->leftHandSide.get();
    switch(lhsNode->nodeType) {
        case ASTNode::NodeType::Identifier: {
            auto idNode = static_cast<IdentifierNode*>(lhsNode);
            ExpressionResult lhs = generateIdentifier(idNode, true);
            if (!lhs.isLValue) {
                reportSemanticError(node->lineNumber, "赋值号左边必须是可修改的左值。");
            }
             if (!checkAssignmentCompatibility(lhs.type, rhs.type, node->lineNumber)) {
                reportSemanticError(node->lineNumber, "赋值类型不兼容: 无法将 '" + rhs.type->name + "' 赋给 '" + lhs.type->name + "'");
            }
            quadruples.push_back(Quadruple("=", rhsPlace, "_", lhs.place));
            break;
        }

        case ASTNode::NodeType::ArrayAccessExpression: {
            auto arrayAccessNode = static_cast<ArrayAccessNode*>(lhsNode);
            ExpressionResult arrayRes = generateExpression(arrayAccessNode->arrayExpr.get());
            ExpressionResult indexRes = generateExpression(arrayAccessNode->indexExpr.get());

            if (arrayRes.type->kind != TypeKind::ARRAY) {
                reportSemanticError(node->lineNumber, "无法对非数组类型进行下标赋值。");
            }
             if (!checkAssignmentCompatibility(arrayRes.type->elementType, rhs.type, node->lineNumber)) {
                reportSemanticError(node->lineNumber, "赋值类型不兼容: 无法将 '" + rhs.type->name + "' 赋给 '" + arrayRes.type->elementType->name + "' 类型的数组成员");
            }

            quadruples.push_back(Quadruple("STORE_AT", rhsPlace, arrayRes.place, indexRes.place));
            break;
        }
        //处理结构体的成员
        case ASTNode::NodeType::MemberAccessExpression: {
            auto memberAccessNode = static_cast<MemberAccessNode*>(lhsNode);

            // 生成结构体变量的表达式
            ExpressionResult baseRes = generateExpression(memberAccessNode->structExpr.get());

            // 语义检查
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

            if (memberOffset == -1) {
                reportSemanticError(node->lineNumber, "结构体 '" + baseRes.type->name + "' 中没有名为 '" + memberAccessNode->memberName + "' 的成员。");
            }

            // 检查赋值类型兼容性
            if (!checkAssignmentCompatibility(memberType, rhs.type, node->lineNumber)) {
                reportSemanticError(node->lineNumber, "赋值类型不兼容: 无法将 '" + rhs.type->name + "' 赋给成员 '" + memberType->name + "'");
            }

            // 生成新的四元式来存储值到成员
            // 我们定义一个新的四元式操作 STORE_MEMBER(src, base, offset)
            quadruples.push_back(Quadruple("STORE_MEMBER", rhsPlace, baseRes.place, to_string(memberOffset)));
            break;
        }
        default:
            reportSemanticError(node->lineNumber, "赋值号左边必须是可修改的左值 (标识符或数组成员)。");
    }
}

void IRGenerator::generateIfStatement(IfStatementNode* node) {
    auto condRes = generateExpression(node->condition.get());
    if (!condRes.isValid() || condRes.type->name != "bool") {
        reportSemanticError(node->condition->lineNumber, "if 条件必须是布尔类型。");
    }
    string elseLabel = symbolTable.generateLabel();
    string endLabel = node->elseBlock ? symbolTable.generateLabel() : elseLabel;
    quadruples.push_back(Quadruple("JUMPF", condRes.place, "_", elseLabel));
    generate(node->thenBlock.get());
    if (node->elseBlock) {
        quadruples.push_back(Quadruple("JUMP", "_", "_", endLabel));
    }
    quadruples.push_back(Quadruple("LABEL", elseLabel, "_", "_"));
    if (node->elseBlock) {
        generate(node->elseBlock.get());
        quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));
    }
}

void IRGenerator::generateWhileStatement(WhileStatementNode* node) {
    string startLabel = symbolTable.generateLabel();
    string endLabel = symbolTable.generateLabel();

    continueLabels.push_back(startLabel);
    breakLabels.push_back(endLabel);

    quadruples.push_back(Quadruple("LABEL", startLabel, "_", "_"));
    auto condRes = generateExpression(node->condition.get());
    if(!condRes.isValid() || condRes.type->name != "bool") reportSemanticError(node->condition->lineNumber, "while 条件必须是布尔类型。");
    quadruples.push_back(Quadruple("JUMPF", condRes.place, "_", endLabel));
    generate(node->loopBlock.get());
    quadruples.push_back(Quadruple("JUMP", "_", "_", startLabel));
    quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));

    continueLabels.pop_back();
    breakLabels.pop_back();
}

void IRGenerator::generatePrintStatement(PrintStatementNode* node) {
    auto exprRes = generateExpression(node->expression.get());
    if (!exprRes.isValid()) {
        reportSemanticError(node->lineNumber, "print 语句中的表达式无效。");
    }
    quadruples.push_back(Quadruple("PRINT", exprRes.place, "_", "_"));
}

ExpressionResult IRGenerator::generateExpression(ASTNode* node, bool needsLValue) {
    if (!node) return ExpressionResult();
    switch (node->nodeType) {
        case ASTNode::NodeType::Literal:      return generateLiteral(static_cast<LiteralNode*>(node));
        case ASTNode::NodeType::Identifier:   return generateIdentifier(static_cast<IdentifierNode*>(node), needsLValue);
        case ASTNode::NodeType::BinaryExpression: return generateBinaryExpression(static_cast<BinaryExpressionNode*>(node));
        case ASTNode::NodeType::UnaryExpression:  return generateUnaryExpression(static_cast<UnaryExpressionNode*>(node));
        case ASTNode::NodeType::FunctionCall: return generateFunctionCall(static_cast<FunctionCallNode*>(node));
        case ASTNode::NodeType::ArrayAccessExpression: return generateArrayAccess(static_cast<ArrayAccessNode*>(node), needsLValue);
        case ASTNode::NodeType::MemberAccessExpression: return generateMemberAccess(static_cast<MemberAccessNode*>(node), needsLValue);
        case ASTNode::NodeType::AssignmentStatement: {
            auto assignNode = static_cast<AssignmentStatementNode*>(node);
            generateAssignmentStatement(assignNode);
            return generateExpression(assignNode->leftHandSide.get(), false);
        }
        case ASTNode::NodeType::InitializerList:
             reportSemanticError(node->lineNumber, "初始化列表不能作为独立的表达式使用。");

        default: reportSemanticError(node->lineNumber, "不支持的表达式节点类型。");
    }
    return ExpressionResult();
}

ExpressionResult IRGenerator::generateFunctionCall(FunctionCallNode* node) {
    if (node->functionExpr->nodeType == ASTNode::NodeType::Identifier) {
        auto funcIdNode = static_cast<IdentifierNode*>(node->functionExpr.get());
        if (funcIdNode->name == "sizeof") {
            if (node->arguments.size() != 1) {
                reportSemanticError(node->lineNumber, "sizeof 函数需要且仅需要一个参数。");
            }
            auto type = getExpressionType(node->arguments[0].get());
            if (!type) {
                 reportSemanticError(node->arguments[0]->lineNumber, "无法确定 sizeof 参数的类型。");
            }
            return ExpressionResult(to_string(type->size), symbolTable.lookupType("int"), false);
        }
    }

    auto funcIdNode = static_cast<IdentifierNode*>(node->functionExpr.get());
    const Symbol* funcSymbol = symbolTable.lookup(funcIdNode->name);
    if (!funcSymbol || funcSymbol->category != SymbolCategory::Function) {
        reportSemanticError(node->lineNumber, "调用的标识符 '" + funcIdNode->name + "' 不是一个函数。");
    }
    auto funcType = funcSymbol->type;
    if (node->arguments.size() != funcType->parameters.size()) {
        reportSemanticError(node->lineNumber, "函数 '" + funcIdNode->name + "' 调用参数数量不匹配。");
    }
    for (int i = node->arguments.size() - 1; i >= 0; --i) {
        auto argRes = generateExpression(node->arguments[i].get());
        if (!checkAssignmentCompatibility(funcType->parameters[i].type, argRes.type, node->arguments[i]->lineNumber)) {
            reportSemanticError(node->arguments[i]->lineNumber, "函数调用中第 " + to_string(i+1) + " 个参数类型不匹配。");
        }
        quadruples.push_back(Quadruple("PARAM", argRes.place, "_", "_"));
    }
    string resultTemp = (funcType->returnType->kind != TypeKind::VOID_TYPE) ? symbolTable.generateTempVar() : "_";
    quadruples.push_back(Quadruple("CALL", funcIdNode->name, to_string(node->arguments.size()), resultTemp));
    return ExpressionResult(resultTemp, funcType->returnType, false);
}

ExpressionResult IRGenerator::generateArrayAccess(ArrayAccessNode* node, bool needsLValue) {
    if (needsLValue) {
        auto arrayRes = generateExpression(node->arrayExpr.get(), false);
        return ExpressionResult(arrayRes.place, arrayRes.type->elementType, true);
    }

    ExpressionResult arrayRes = generateExpression(node->arrayExpr.get());
    ExpressionResult indexRes = generateExpression(node->indexExpr.get());
    if (!arrayRes.isValid() || arrayRes.type->kind != TypeKind::ARRAY) {
        reportSemanticError(node->lineNumber, "试图对非数组类型进行下标访问。");
    }
    if (!indexRes.isValid() || indexRes.type->name != "int") {
        reportSemanticError(node->lineNumber, "数组索引必须是整数类型。");
    }
    auto elementType = arrayRes.type->elementType;
    string resultTemp = symbolTable.generateTempVar();
    quadruples.push_back(Quadruple("LOAD_AT", resultTemp, arrayRes.place, indexRes.place));
    return ExpressionResult(resultTemp, elementType, false);
}

ExpressionResult IRGenerator::generateBinaryExpression(BinaryExpressionNode* node) {
    auto lhs = generateExpression(node->left.get());
    auto rhs = generateExpression(node->right.get());
    auto resultType = checkOperationType(lhs.type, rhs.type, node->op, node->lineNumber);
    if (!resultType) {
        string lhs_name = lhs.type ? lhs.type->name : "无效类型";
        string rhs_name = rhs.type ? rhs.type->name : "无效类型";
        reportSemanticError(node->lineNumber, "二元操作符 '" + node->op + "' 的操作数类型不兼容 (" + lhs_name + ", " + rhs_name + ")");
    }
    string tempVar = symbolTable.generateTempVar();
    quadruples.push_back(Quadruple(node->op, lhs.place, rhs.place, tempVar));
    return ExpressionResult(tempVar, resultType, false);
}

ExpressionResult IRGenerator::generateUnaryExpression(UnaryExpressionNode* node) {
    auto operandRes = generateExpression(node->operand.get());
    auto resultType = checkOperationType(operandRes.type, nullptr, node->op, node->lineNumber);
    if (!resultType) {
        string type_name = operandRes.type ? operandRes.type->name : "无效类型";
        reportSemanticError(node->lineNumber, "一元操作符 '" + node->op + "' 不支持类型 " + type_name);
    }
    string tempVar = symbolTable.generateTempVar();
    quadruples.push_back(Quadruple(node->op, operandRes.place, "_", tempVar));
    return ExpressionResult(tempVar, resultType, false);
}

ExpressionResult IRGenerator::generateLiteral(LiteralNode* node) {
    string typeName;
    switch (node->literalType) {
        case TokenType::INT_LITERAL:    typeName = "int"; break;
        case TokenType::FLOAT_LITERAL:  typeName = "float"; break;
        case TokenType::STRING_LITERAL: typeName = "string"; break;
        case TokenType::CHAR_LITERAL:   typeName = "char"; break;
        case TokenType::KW_TRUE:
        case TokenType::KW_FALSE:       typeName = "bool"; break;
        default: reportSemanticError(node->lineNumber, "未知的字面量类型。");
    }
    auto typeInfo = symbolTable.lookupType(typeName);
    if (!typeInfo) {
        reportSemanticError(node->lineNumber, "内部错误：在符号表中找不到基础类型 '" + typeName + "'。");
    }
    return ExpressionResult(node->value, typeInfo, false);
}

ExpressionResult IRGenerator::generateIdentifier(IdentifierNode* node, bool needsLValue) {
    const Symbol* sym = symbolTable.lookup(node->name);
    if(!sym) {
        reportSemanticError(node->lineNumber, "未声明的标识符: " + node->name);
    }
    if (sym->category == SymbolCategory::Function) {
        reportSemanticError(node->lineNumber, "函数名 '" + node->name + "' 只能用于函数调用。");
    }
    bool isLVal = (sym->category == SymbolCategory::Variable);
    return ExpressionResult(node->name, sym->type, isLVal);
}

ExpressionResult IRGenerator::generateMemberAccess(MemberAccessNode* node, bool needsLValue) {
    // 步骤 1: 生成结构体变量本身的表达式，获取其位置和类型
    ExpressionResult baseRes = generateExpression(node->structExpr.get());

    // 步骤 2: 语义检查 - 确保是在对一个结构体类型进行操作
    if (!baseRes.isValid() || baseRes.type->kind != TypeKind::STRUCT) {
        reportSemanticError(node->lineNumber, "点运算符(.)只能用于结构体类型。");
    }

    // 步骤 3: 在结构体的类型信息中查找成员，以获取其偏移量和类型
    int memberOffset = -1;
    shared_ptr<TypeInfo> memberType = nullptr;
    for (const auto& member : baseRes.type->structMembers) {
        if (member.name == node->memberName) {
            memberOffset = member.offset;
            memberType = member.type;
            break;
        }
    }

    if (memberOffset == -1) {
        reportSemanticError(node->lineNumber, "结构体 '" + baseRes.type->name + "' 中没有名为 '" + node->memberName + "' 的成员。");
    }

    // 步骤 4: 生成新的四元式来加载成员的值
    // 我们定义一个新的四元式操作 LOAD_MEMBER(dest, base, offset)
    string resultTemp = symbolTable.generateTempVar();
    quadruples.push_back(Quadruple("LOAD_MEMBER", resultTemp, baseRes.place, to_string(memberOffset)));

    // 返回一个代表成员值的临时变量（R-value）
    return ExpressionResult(resultTemp, memberType, false);
}

std::shared_ptr<TypeInfo> IRGenerator::getExpressionType(ASTNode* node) {
    if (!node) return nullptr;
     if (node->nodeType == ASTNode::NodeType::Identifier) {
        auto idNode = static_cast<IdentifierNode*>(node);
        const Symbol* symbol = symbolTable.lookup(idNode->name);
        if (symbol) return symbol->type;
    }
    return nullptr;
}

bool IRGenerator::checkAssignmentCompatibility(const std::shared_ptr<TypeInfo>& target, const std::shared_ptr<TypeInfo>& source, int line) {
    if (!target || !source || target->kind == TypeKind::UNKNOWN || source->kind == TypeKind::UNKNOWN) return false;
    if (target->kind == TypeKind::VOID_TYPE || source->kind == TypeKind::VOID_TYPE) return false;
    if (target->name == source->name) return true;
    if (target->name == "float" && source->name == "int") return true;
    return false;
}

std::shared_ptr<TypeInfo> IRGenerator::checkOperationType(const std::shared_ptr<TypeInfo>& type1, const std::shared_ptr<TypeInfo>& type2, const std::string& op, int line) {
    if (!type1) return nullptr;

    // --- 处理二元运算符 ---
    if (type2) {
        // 规则 1: 字符串拼接
        if (op == "+") {
            // 如果操作符是+，且任意一方是字符串，另一方是任意基本类型，则为合法的字符串拼接
            if ((type1->name == "string" && type2->kind == TypeKind::PRIMITIVE) ||
                (type2->name == "string" && type1->kind == TypeKind::PRIMITIVE)) {
                return symbolTable.lookupType("string");
                }
        }

        // 规则 2: 数字算术运算
        if ((op == "+" || op == "-" || op == "*" || op == "/") &&
            (type1->name == "int" || type1->name == "float") &&
            (type2->name == "int" || type2->name == "float")) {
            if (type1->name == "float" || type2->name == "float") return symbolTable.lookupType("float");
            return symbolTable.lookupType("int");
            }

        // 规则 3: 比较运算
        if (op == ">" || op == "<" || op == ">=" || op == "<=" || op == "==" || op == "!=") {
            if (type1->name == type2->name && type1->kind == TypeKind::PRIMITIVE) {
                return symbolTable.lookupType("bool");
            }
        }

        // 规则 4: 逻辑运算
        if ((op == "&&" || op == "||") && type1->name == "bool" && type2->name == "bool") {
            return symbolTable.lookupType("bool");
        }
    }
    // 处理一元运算符
    else {
        if(op == "!") {
            if (type1->name == "bool") return symbolTable.lookupType("bool");
        }
        if(op == "-") {
            if (type1->name == "int" || type1->name == "float") return type1;
        }
    }

    // 如果没有任何规则匹配，则操作非法
    return nullptr;
}

void IRGenerator::generateForStatement(ForStatementNode* node) {
    // 步骤 1: 为 for 循环创建并进入一个新的作用域，以支持循环内变量声明。
    symbolTable.enterScope();

    // 步骤 2: 生成初始化语句的四元式。
    if (node->initialization) {
        generate(node->initialization.get());
    }

    // 步骤 3: 创建循环控制所需的标签。
    string conditionLabel = symbolTable.generateLabel(); // 条件判断的入口
    string incrementLabel = symbolTable.generateLabel(); // continue 语句的跳转目标
    string endLabel = symbolTable.generateLabel();       // break 语句和循环退出的跳转目标

    // 步骤 4: 将 'break' 和 'continue' 的目标标签压入栈中，供循环体内的语句使用。
    breakLabels.push_back(endLabel);
    continueLabels.push_back(incrementLabel);

    // 步骤 5: 放置条件判断标签，作为循环的起点。
    quadruples.push_back(Quadruple("LABEL", conditionLabel, "_", "_"));

    // 步骤 6: 生成条件判断和跳转逻辑。
    if (node->condition) {
        ExpressionResult condRes = generateExpression(node->condition.get());
        if (!condRes.isValid() || condRes.type->name != "bool") {
            reportSemanticError(node->condition->lineNumber, "for 循环的条件必须是布尔类型。");
        }
        // 如果条件为假，则跳转到循环结束标签。
        quadruples.push_back(Quadruple("JUMPF", condRes.place, "_", endLabel));
    }
    // 注：如果条件部分为空，则视为无限循环，不生成跳转指令。

    // 步骤 7: 生成循环体的四元式。
    if (node->body) {
        generate(node->body.get());
    }

    // 步骤 8: 放置增量语句标签。
    quadruples.push_back(Quadruple("LABEL", incrementLabel, "_", "_"));

    // 步骤 9: 生成增量语句的四元式。
    if (node->increment) {
        // 调用 generateExpression 是因为增量部分通常是带有副作用的表达式，我们关心其执行过程而非结果。
        generateExpression(node->increment.get());
    }

    // 步骤 10: 生成无条件跳转，返回到条件判断部分，形成循环。
    quadruples.push_back(Quadruple("JUMP", "_", "_", conditionLabel));

    // 步骤 11: 放置循环结束标签。
    quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));

    // 步骤 12: 循环结束，从栈中弹出 'break' 和 'continue' 的上下文。
    continueLabels.pop_back();
    breakLabels.pop_back();

    // 步骤 13: 退出 for 循环的作用域。
    symbolTable.exitScope();
}

void IRGenerator::generateStructiDefinition(StructiDefinitionNode* node) {
    // 检查结构体是否已被定义
    if (symbolTable.lookupType(node->structiName) != nullptr) {
        reportSemanticError(node->lineNumber, "结构体 '" + node->structiName + "' 重复定义。");
        return;
    }

    // 创建一个新的 TypeInfo 用于描述该结构体
    auto structType = make_shared<TypeInfo>(TypeKind::STRUCT, node->structiName, 0);
    int currentOffset = 0;
    int totalSize = 0;

    // 遍历所有成员声明，计算偏移量和总大小
    for (const auto& memberNode : node->memberDeclarations->statements) {
        auto declNode = static_cast<DeclarationStatementNode*>(memberNode.get());
        auto memberType = getTypeFromNode(declNode->typeSpecifier.get());
        if (!memberType) {
            reportSemanticError(declNode->lineNumber, "结构体成员 '" + declNode->identifierName + "' 的类型未知。");
            continue;
        }

        // 将成员信息（名称、类型、偏移量）存入 TypeInfo
        structType->structMembers.push_back({declNode->identifierName, memberType, currentOffset});

        // 更新偏移量和总大小 (此处假设所有类型大小已知)
        // 注意：为简化，我们假设所有基本类型和指针都占用2字节(WORD)
        int memberSize = 2; // 默认大小为 WORD
        // 更复杂的实现会从 memberType->size 获取大小
        currentOffset += memberSize;
        totalSize += memberSize;
    }

    structType->size = totalSize; // 更新结构体的总大小

    // 将完整定义的结构体类型添加到符号表中，供后续使用
    symbolTable.addType(node->structiName, structType);
}

void IRGenerator::generateSwitchStatement(SwitchStatementNode* node) {
    ExpressionResult switchExpr = generateExpression(node->expression.get());
    if (switchExpr.type->name != "int" && switchExpr.type->name != "char") {
        reportSemanticError(node->lineNumber, "switch 语句的表达式必须是整数或字符类型。");
    }

    string endLabel = symbolTable.generateLabel();
    string defaultLabel = "";
    vector<pair<string, string>> caseLabels;

    for (const auto& caseNode : node->cases) {
        if (caseNode->value) {
            string caseBodyLabel = symbolTable.generateLabel();
            caseLabels.push_back({caseBodyLabel, ""});

            ExpressionResult caseValue = generateExpression(caseNode->value.get());
            if (!checkAssignmentCompatibility(switchExpr.type, caseValue.type, caseNode->lineNumber)) {
                reportSemanticError(caseNode->lineNumber, "case 标签的类型与 switch 表达式的类型不匹配。");
            }
            string tempVar = symbolTable.generateTempVar();
            quadruples.push_back(Quadruple("==", switchExpr.place, caseValue.place, tempVar));
            quadruples.push_back(Quadruple("JUMPNZ", tempVar, "_", caseBodyLabel));
        } else {
            if (!defaultLabel.empty()) {
                reportSemanticError(caseNode->lineNumber, "一个 switch 语句中只能有一个 default 标签。");
            }
            defaultLabel = symbolTable.generateLabel();
        }
    }

    if (!defaultLabel.empty()) {
        quadruples.push_back(Quadruple("JUMP", "_", "_", defaultLabel));
    } else {
        quadruples.push_back(Quadruple("JUMP", "_", "_", endLabel));
    }

    breakLabels.push_back(endLabel);
    int caseIndex = 0;
    for (const auto& caseNode : node->cases) {
        if (caseNode->value) {
            quadruples.push_back(Quadruple("LABEL", caseLabels[caseIndex].first, "_", "_"));
            generateStatementList(caseNode->body.get());
            caseIndex++;
        } else {
            quadruples.push_back(Quadruple("LABEL", defaultLabel, "_", "_"));
            generateStatementList(caseNode->body.get());
        }
    }
    breakLabels.pop_back();

    quadruples.push_back(Quadruple("LABEL", endLabel, "_", "_"));
}

void IRGenerator::generateBreakStatement(BreakStatementNode* node) {
    if (breakLabels.empty()) {
        reportSemanticError(node->lineNumber, "break 语句只能出现在循环或switch语句中。");
    }
    quadruples.push_back(Quadruple("JUMP", "_", "_", breakLabels.back()));
}

void IRGenerator::generateContinueStatement(ContinueStatementNode* node) {
    if (continueLabels.empty()) {
        reportSemanticError(node->lineNumber, "continue 语句只能出现在循环语句中。");
    }
    quadruples.push_back(Quadruple("JUMP", "_", "_", continueLabels.back()));
}

void IRGenerator::dumpQuadruples() const {
    cout << "\n--- 生成的四元式 ---" << endl;
    for (size_t i = 0; i < quadruples.size(); ++i) {
        cout << i << ":\t" << quadruples[i].toString() << endl;
    }
    cout << "--- 四元式结束 ---" << endl;
}