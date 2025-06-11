#include "ast_nodes.h"
#include "parser.h" // For tokenTypeToString
#include <iostream>

using namespace std;

// 内部辅助函数：用于打印指定数量的缩进空格
void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        cout << "  ";
    }
}

// --- ASTNode 基类 ---
void ASTNode::print(int indent) const {
    printIndent(indent);
    cout << "ASTNode (节点类型: " << static_cast<int>(nodeType) << ", 行号: " << lineNumber << ")" << endl;
}

// --- InitializerListNode 的 print 实现 ---
void InitializerListNode::print(int indent) const {
    printIndent(indent);
    cout << "InitializerListNode (初始化列表, 行号: " << lineNumber << ", 元素数量: " << elements.size() << ")" << endl;
    for(const auto& elem : elements) {
        if(elem) {
            elem->print(indent + 1);
        }
    }
}

// --- Program & Statement Nodes ---
void ProgramNode::print(int indent) const {
    printIndent(indent);
    cout << "ProgramNode (程序根节点, 行号: " << lineNumber << ")" << endl;
    if (statementList) {
        statementList->print(indent + 1);
    }
}

void StatementListNode::print(int indent) const {
    printIndent(indent);
    cout << "StatementListNode (语句列表, 行号: " << lineNumber << ", 语句数量: " << statements.size() << ")" << endl;
    for (const auto& stmt : statements) {
        if (stmt) {
            stmt->print(indent + 1);
        }
    }
}

void DeclarationStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "DeclarationStatementNode (声明语句, 标识符: " << identifierName << ", 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "类型说明符: " << endl;
    if (typeSpecifier) {
        typeSpecifier->print(indent + 2);
    }

    if (initialValue) {
        printIndent(indent + 1);
        cout << "初始化值: " << endl;
        initialValue->print(indent + 2);
    }
}

void AssignmentStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "AssignmentStatementNode (赋值语句, 运算符: " << op << ", 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "左值 (LHS): " << endl;
    if (leftHandSide) {
        leftHandSide->print(indent + 2);
    }

    printIndent(indent + 1);
    cout << "赋值表达式 (RHS): " << endl;
    if (expression) {
        expression->print(indent + 2);
    }
}

void IfStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "IfStatementNode (If语句, 行号: " << lineNumber << ")" << endl;
    printIndent(indent + 1); cout << "条件: " << endl;
    if (condition) condition->print(indent + 2);
    printIndent(indent + 1); cout << "Then语句块: " << endl;
    if (thenBlock) thenBlock->print(indent + 2);
    if (elseBlock) {
        printIndent(indent + 1); cout << "Else语句块: " << endl;
        elseBlock->print(indent + 2);
    }
}

void WhileStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "WhileStatementNode (While语句, 行号: " << lineNumber << ")" << endl;
    printIndent(indent + 1); cout << "循环条件: " << endl;
    if (condition) condition->print(indent + 2);
    printIndent(indent + 1); cout << "循环体: " << endl;
    if (loopBlock) loopBlock->print(indent + 2);
}

void ForStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "ForStatementNode (For语句, 行号: " << lineNumber << ")" << endl;
    printIndent(indent + 1); cout << "初始化部分: " << endl;
    if (initialization) initialization->print(indent + 2); else { printIndent(indent + 2); cout << "<空>" << endl; }
    printIndent(indent + 1); cout << "条件部分: " << endl;
    if (condition) condition->print(indent + 2); else { printIndent(indent + 2); cout << "<空 (默认为true)>" << endl; }
    printIndent(indent + 1); cout << "迭代表达式部分: " << endl;
    if (increment) increment->print(indent + 2); else { printIndent(indent + 2); cout << "<空>" << endl; }
    printIndent(indent + 1); cout << "循环体: " << endl;
    if (body) body->print(indent + 2);
}

void PrintStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "PrintStatementNode (Print语句, 行号: " << lineNumber << ")" << endl;
    printIndent(indent + 1); cout << "打印表达式: " << endl;
    if (expression) expression->print(indent + 2);
}

void TypeNode::print(int indent) const {
    printIndent(indent);
    cout << "TypeNode (类型节点): " << typeName << " (行号: " << lineNumber << ")" << endl;
}

void StructiDefinitionNode::print(int indent) const {
    printIndent(indent);
    cout << "StructiDefinitionNode (Structi 定义, 名称: " << structiName << ", 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "成员列表: " << endl;
    if (memberDeclarations) {
        memberDeclarations->print(indent + 2);
    }
}

void ArrayTypeNode::print(int indent) const {
    printIndent(indent);
    cout << "ArrayTypeNode (数组类型, 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "元素类型: " << endl;
    if (elementType) elementType->print(indent + 2);

    if (sizeExpression) {
        printIndent(indent + 1);
        cout << "大小表达式: " << endl;
        sizeExpression->print(indent + 2);
    } else {
        printIndent(indent + 1);
        cout << "大小: 动态" << endl;
    }
}


void MemberAccessNode::print(int indent) const {
    printIndent(indent);
    cout << "MemberAccessNode (成员访问, 成员名: " << memberName << ", 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "结构体表达式: " << endl;
    if (structExpr) structExpr->print(indent + 2);
}

void ArrayAccessNode::print(int indent) const {
    printIndent(indent);
    cout << "ArrayAccessNode (数组访问, 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "数组表达式: " << endl;
    if (arrayExpr) arrayExpr->print(indent + 2);

    printIndent(indent + 1);
    cout << "索引表达式: " << endl;
    if (indexExpr) indexExpr->print(indent + 2);
}

void BinaryExpressionNode::print(int indent) const {
    printIndent(indent);
    cout << "BinaryExpressionNode (运算符: " << op << ", 行号: " << lineNumber << ")" << endl;
    printIndent(indent + 1); cout << "左操作数: " << endl;
    if (left) left->print(indent + 2);
    printIndent(indent + 1); cout << "右操作数: " << endl;
    if (right) right->print(indent + 2);
}

void UnaryExpressionNode::print(int indent) const {
    printIndent(indent);
    cout << "UnaryExpressionNode (运算符: " << op << ", 行号: " << lineNumber << ")" << endl;
    printIndent(indent + 1); cout << "操作数: " << endl;
    if (operand) operand->print(indent + 2);
}

void LiteralNode::print(int indent) const {
    printIndent(indent);
    cout << "LiteralNode (类型: " << tokenTypeToString(literalType) << "): \"" << value << "\" (行号: " << lineNumber << ")" << endl;
}

void IdentifierNode::print(int indent) const {
    printIndent(indent);
    cout << "IdentifierNode (标识符): \"" << name << "\" (行号: " << lineNumber << ")" << endl;
}


void FunctionDefinitionNode::print(int indent) const {
    printIndent(indent);
    cout << "FunctionDefinitionNode (函数定义: " << functionName << ", 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "返回类型: " << endl;
    if(returnType) returnType->print(indent + 2);

    printIndent(indent + 1);
    cout << "参数列表: " << endl;
    if(parameters && !parameters->statements.empty()) {
        parameters->print(indent + 2);
    } else {
        printIndent(indent + 2);
        cout << "<无参数>" << endl;
    }

    printIndent(indent + 1);
    cout << "函数体: " << endl;
    if(body) body->print(indent + 2);
}

void FunctionCallNode::print(int indent) const {
    printIndent(indent);
    cout << "FunctionCallNode (函数调用, 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1);
    cout << "函数名表达式: " << endl;
    if(functionExpr) functionExpr->print(indent + 2);

    printIndent(indent + 1);
    cout << "参数: " << endl;
    if (!arguments.empty()) {
        for (const auto& arg : arguments) {
            arg->print(indent + 2);
        }
    } else {
        printIndent(indent + 2);
        cout << "<无参数>" << endl;
    }
}

void ReturnStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "ReturnStatementNode (返回语句, 行号: " << lineNumber << ")" << endl;

    if (returnValue) {
        printIndent(indent + 1);
        cout << "返回值表达式: " << endl;
        returnValue->print(indent + 2);
    } else {
        printIndent(indent + 1);
        cout << "<void 返回>" << endl;
    }
}


void BreakStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "BreakStatementNode (Break语句, 行号: " << lineNumber << ")" << endl;
}

void ContinueStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "ContinueStatementNode (Continue语句, 行号: " << lineNumber << ")" << endl;
}

void CaseStatementNode::print(int indent) const {
    printIndent(indent);
    if (value) {
        cout << "CaseStatementNode (行号: " << lineNumber << ")" << endl;
        printIndent(indent + 1); cout << "匹配值: " << endl;
        value->print(indent + 2);
    } else {
        cout << "DefaultStatementNode (行号: " << lineNumber << ")" << endl;
    }
    printIndent(indent + 1); cout << "执行体: " << endl;
    if (body) body->print(indent + 2);
}

void SwitchStatementNode::print(int indent) const {
    printIndent(indent);
    cout << "SwitchStatementNode (Switch语句, 行号: " << lineNumber << ")" << endl;

    printIndent(indent + 1); cout << "判断表达式: " << endl;
    if (expression) expression->print(indent + 2);

    printIndent(indent + 1); cout << "分支列表: " << endl;
    for (const auto& case_stmt : cases) {
        case_stmt->print(indent + 2);
    }
}
