// ast_nodes.h
#ifndef AST_NODES_H
#define AST_NODES_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "token.h"

class StatementListNode;
class TypeNode;

//基类
class ASTNode {
public:
    enum NodeType {//所有的语法结构，好吧有点乱，防止错误还是保持这样吧
        Program,//root
        StatementList,//语句块，语句列表，整个函数（块）
        DeclarationStatement,//变量或者常量的声明：int a；
        AssignmentStatement,//赋值语句 a=1
        IfStatement,//if-else的条件语句
        WhileStatement,//同理
        ForStatement,
        StructiDefinitionStatement,
        MemberAccessExpression,//结构体成员访问
        ArrayType,//数组类型
        ArrayAccessExpression,//数组的成员访问access
        PrintStatement,
        BinaryExpression,//二元表达式，加减乘除等等
        UnaryExpression,//一元表达式
        Literal,//常量：整数，字符串，布尔型
        Identifier,//标识符
        Type,//基本类型或者结构体名
        FunctionDefinition,//函数定义完全体
        FunctionCall,//函数调用
        ReturnStatement,
        SwitchStatement,
        CaseStatement,
        BreakStatement,
        ContinueStatement,
        InitializerList//初始化列表
    };
    NodeType nodeType;//枚举类型，未来会强制变换为整型
    int lineNumber; //行号

public:
    ASTNode(NodeType type, int line) : nodeType(type), lineNumber(line) {}
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const;
};

//初始化列表类
class InitializerListNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> elements;

    InitializerListNode(std::vector<std::unique_ptr<ASTNode>> elems, int line)
        : ASTNode(NodeType::InitializerList, line), elements(std::move(elems)) {}

    void print(int indent = 0) const override;
};

//根结点类
class ProgramNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> statementList;
    ProgramNode(std::unique_ptr<ASTNode> stmtList, int line)
        : ASTNode(NodeType::Program, line), statementList(std::move(stmtList)) {}
    void print(int indent = 0) const override;
};

//语句块类，就是被大括号围起来的类
class StatementListNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    StatementListNode(int line) : ASTNode(NodeType::StatementList, line) {}
    void addStatement(std::unique_ptr<ASTNode> stmt) {
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    void print(int indent = 0) const override;
};

//基本数据类型的名字，比如int，float还有自己命名的结构体
class TypeNode : public ASTNode {
public:
    std::string typeName;
    TypeNode(const std::string& name, int line)
        : ASTNode(NodeType::Type, line), typeName(name) {}
    void print(int indent = 0) const override;
};

//数组类型存储
class ArrayTypeNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> elementType;//数组类型
    std::unique_ptr<ASTNode> sizeExpression;//大小，动态数组设为空

    ArrayTypeNode(std::unique_ptr<ASTNode> elemType, std::unique_ptr<ASTNode> sizeExpr, int line)
        : ASTNode(NodeType::ArrayType, line),
          elementType(std::move(elemType)), sizeExpression(std::move(sizeExpr)) {}

    void print(int indent = 0) const override;
};

//变量语句存储
class DeclarationStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> typeSpecifier;//类型，可以是typenode也可以是arraynode
    std::string identifierName;//名字
    std::unique_ptr<ASTNode> initialValue;//初值

    DeclarationStatementNode(std::unique_ptr<ASTNode> typeSpec, const std::string& idName,
                             std::unique_ptr<ASTNode> initVal, int line)
        : ASTNode(NodeType::DeclarationStatement, line), typeSpecifier(std::move(typeSpec)),
          identifierName(idName), initialValue(std::move(initVal)) {}
    void print(int indent = 0) const override;
};

class ArrayAccessNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> arrayExpr;
    std::unique_ptr<ASTNode> indexExpr;

    ArrayAccessNode(std::unique_ptr<ASTNode> arrExpr, std::unique_ptr<ASTNode> idxExpr, int line)
        : ASTNode(NodeType::ArrayAccessExpression, line),
          arrayExpr(std::move(arrExpr)), indexExpr(std::move(idxExpr)) {}

    void print(int indent = 0) const override;
};

class StructiDefinitionNode : public ASTNode {
public:
    std::string structiName;
    std::unique_ptr<StatementListNode> memberDeclarations;
    StructiDefinitionNode(const std::string& name, std::unique_ptr<StatementListNode> members, int line)
        : ASTNode(NodeType::StructiDefinitionStatement, line),
          structiName(name), memberDeclarations(std::move(members)) {}
    void print(int indent = 0) const override;
};

//点操作符，成员访问
class MemberAccessNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> structExpr;
    std::string memberName;
    MemberAccessNode(std::unique_ptr<ASTNode> sExpr, const std::string& mName, int line)
        : ASTNode(NodeType::MemberAccessExpression, line),
          structExpr(std::move(sExpr)), memberName(mName) {}
    void print(int indent = 0) const override;
};

//赋值语句
class AssignmentStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> leftHandSide;
    std::string op;
    std::unique_ptr<ASTNode> expression;

    AssignmentStatementNode(std::unique_ptr<ASTNode> lhs, std::string oper, std::unique_ptr<ASTNode> expr, int line)
        : ASTNode(NodeType::AssignmentStatement, line), leftHandSide(std::move(lhs)), op(std::move(oper)), expression(std::move(expr)) {}

    AssignmentStatementNode(std::unique_ptr<ASTNode> lhs, std::unique_ptr<ASTNode> expr, int line)
        : ASTNode(NodeType::AssignmentStatement, line), leftHandSide(std::move(lhs)), op("="), expression(std::move(expr)) {}


    void print(int indent = 0) const override;
};

class IfStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBlock;
    std::unique_ptr<ASTNode> elseBlock;
    IfStatementNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> thenB,
                    std::unique_ptr<ASTNode> elseB, int line)
        : ASTNode(NodeType::IfStatement, line), condition(std::move(cond)),
          thenBlock(std::move(thenB)), elseBlock(std::move(elseB)) {}
    void print(int indent = 0) const override;
};

class WhileStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> loopBlock;
    WhileStatementNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> loopB, int line)
        : ASTNode(NodeType::WhileStatement, line), condition(std::move(cond)), loopBlock(std::move(loopB)) {}
    void print(int indent = 0) const override;
};

class ForStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> initialization;
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> increment;
    std::unique_ptr<StatementListNode> body;
    ForStatementNode(std::unique_ptr<ASTNode> init,
                     std::unique_ptr<ASTNode> cond,
                     std::unique_ptr<ASTNode> incr,
                     std::unique_ptr<StatementListNode> b, int line)
        : ASTNode(NodeType::ForStatement, line),
          initialization(std::move(init)),
          condition(std::move(cond)),
          increment(std::move(incr)),
          body(std::move(b)) {}
    void print(int indent = 0) const override;
};

class PrintStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;
    PrintStatementNode(std::unique_ptr<ASTNode> expr, int line)
        : ASTNode(NodeType::PrintStatement, line), expression(std::move(expr)) {}
    void print(int indent = 0) const override;
};

class BinaryExpressionNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> left;
    std::string op;
    std::unique_ptr<ASTNode> right;
    BinaryExpressionNode(std::unique_ptr<ASTNode> l, const std::string& oper, std::unique_ptr<ASTNode> r, int line)
        : ASTNode(NodeType::BinaryExpression, line), left(std::move(l)), op(oper), right(std::move(r)) {}
    void print(int indent = 0) const override;
};

class UnaryExpressionNode : public ASTNode {
public:
    std::string op;
    std::unique_ptr<ASTNode> operand;
    UnaryExpressionNode(const std::string& oper, std::unique_ptr<ASTNode> opnd, int line)
        : ASTNode(NodeType::UnaryExpression, line), op(oper), operand(std::move(opnd)) {}
    void print(int indent = 0) const override;
};

class LiteralNode : public ASTNode {
public:
    std::string value;
    TokenType literalType;

    LiteralNode(const std::string& val, TokenType type, int line)
        : ASTNode(NodeType::Literal, line), value(val), literalType(type) {}
    void print(int indent = 0) const override;
};


class IdentifierNode : public ASTNode {
public:
    std::string name;
    IdentifierNode(const std::string& idName, int line)
        : ASTNode(NodeType::Identifier, line), name(idName) {}
    void print(int indent = 0) const override;
};

class FunctionDefinitionNode : public ASTNode {
public:
    std::string functionName;
    std::unique_ptr<TypeNode> returnType;
    std::unique_ptr<StatementListNode> parameters;
    std::unique_ptr<StatementListNode> body;

    FunctionDefinitionNode(std::string name, std::unique_ptr<TypeNode> retType,
                             std::unique_ptr<StatementListNode> params, std::unique_ptr<StatementListNode> b, int line)
        : ASTNode(NodeType::FunctionDefinition, line), functionName(std::move(name)),
          returnType(std::move(retType)), parameters(std::move(params)), body(std::move(b)) {}

    void print(int indent = 0) const override;
};

class FunctionCallNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> functionExpr;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    FunctionCallNode(std::unique_ptr<ASTNode> func, std::vector<std::unique_ptr<ASTNode>> args, int line)
        : ASTNode(NodeType::FunctionCall, line),
          functionExpr(std::move(func)), arguments(std::move(args)) {}

    void print(int indent = 0) const override;
};

class ReturnStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> returnValue;

    ReturnStatementNode(std::unique_ptr<ASTNode> retVal, int line)
        : ASTNode(NodeType::ReturnStatement, line), returnValue(std::move(retVal)) {}

    void print(int indent = 0) const override;
};

class BreakStatementNode : public ASTNode {
public:
    BreakStatementNode(int line) : ASTNode(NodeType::BreakStatement, line) {}
    void print(int indent = 0) const override;
};

class ContinueStatementNode : public ASTNode {
public:
    ContinueStatementNode(int line) : ASTNode(NodeType::ContinueStatement, line) {}
    void print(int indent = 0) const override;
};

class CaseStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> value;
    std::unique_ptr<StatementListNode> body;

    CaseStatementNode(std::unique_ptr<ASTNode> val, std::unique_ptr<StatementListNode> b, int line)
        : ASTNode(NodeType::CaseStatement, line), value(std::move(val)), body(std::move(b)) {}

    void print(int indent = 0) const override;
};

class SwitchStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;
    std::vector<std::unique_ptr<CaseStatementNode>> cases;

    SwitchStatementNode(std::unique_ptr<ASTNode> expr, std::vector<std::unique_ptr<CaseStatementNode>> caseList, int line)
        : ASTNode(NodeType::SwitchStatement, line), expression(std::move(expr)), cases(std::move(caseList)) {}
    void print(int indent = 0) const override;
};


#endif // AST_NODES_H
