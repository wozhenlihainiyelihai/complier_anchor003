#include "ast_nodes.h"
#include "parser.h"
#include <iostream>

using namespace std;

// 辅助：打印指定数量的indent缩进空格（为了ast生成更加美观……好吧其实没有什么大用，但是写了就不想删了🤣）
void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        cout << "  ";
    }
}

//下面是众多print实现：基本都是三个模式：1. 打印自身信息 2. 递归子节点 3. 处理空指针，如果空就会推出，避免崩溃
// ASTNode 基类：如果派生类忘记override自己的print方法，调用就会执行这个版本。
void ASTNode::print(int indent) const {
    printIndent(indent);
    cout << "ASTNode (节点类型: " << static_cast<int>(nodeType) << ", 行号: " << lineNumber << ")" << endl;
}

void InitializerListNode::print(int indent) const {
    printIndent(indent);
    cout << "InitializerListNode (初始化列表, 行号: " << lineNumber << ", 元素数量: " << elements.size() << ")" << endl;
    for(const auto& elem : elements) {//unique_ptr保证其作用范围，不会收到其他函数影响
        if(elem) {
            elem->print(indent + 1);
        }
    }
}

//打印根结点
void ProgramNode::print(int indent) const {
    printIndent(indent);
    cout << "ProgramNode (程序根节点, 行号: " << lineNumber << ")" << endl;
    if (statementList) { //如果子节点不是空的
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
