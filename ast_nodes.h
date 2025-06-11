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

class ASTNode {
public:
    enum NodeType {
        Program,
        StatementList,
        DeclarationStatement,
        AssignmentStatement,
        IfStatement,
        WhileStatement,
        ForStatement,
        StructiDefinitionStatement,
        MemberAccessExpression,
        ArrayType,
        ArrayAccessExpression,
        PrintStatement,
        BinaryExpression,
        UnaryExpression,
        Literal,
        Identifier,
        Type,
        FunctionDefinition,
        FunctionCall,
        ReturnStatement,
        SwitchStatement,
        CaseStatement,
        BreakStatement,
        ContinueStatement,
        InitializerList
    };

    NodeType nodeType;
    int lineNumber;

    ASTNode(NodeType type, int line) : nodeType(type), lineNumber(line) {}
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const;
};

class InitializerListNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> elements;

    InitializerListNode(std::vector<std::unique_ptr<ASTNode>> elems, int line)
        : ASTNode(NodeType::InitializerList, line), elements(std::move(elems)) {}

    void print(int indent = 0) const override;
};


class ProgramNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> statementList;
    ProgramNode(std::unique_ptr<ASTNode> stmtList, int line)
        : ASTNode(NodeType::Program, line), statementList(std::move(stmtList)) {}
    void print(int indent = 0) const override;
};

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

class TypeNode : public ASTNode {
public:
    std::string typeName;
    TypeNode(const std::string& name, int line)
        : ASTNode(NodeType::Type, line), typeName(name) {}
    void print(int indent = 0) const override;
};

// --- 修改: elementType 现在是 ASTNode* 以支持嵌套 ---
class ArrayTypeNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> elementType;
    std::unique_ptr<ASTNode> sizeExpression;

    ArrayTypeNode(std::unique_ptr<ASTNode> elemType, std::unique_ptr<ASTNode> sizeExpr, int line)
        : ASTNode(NodeType::ArrayType, line),
          elementType(std::move(elemType)), sizeExpression(std::move(sizeExpr)) {}

    void print(int indent = 0) const override;
};

class DeclarationStatementNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> typeSpecifier;
    std::string identifierName;
    std::unique_ptr<ASTNode> initialValue;

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

class MemberAccessNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> structExpr;
    std::string memberName;
    MemberAccessNode(std::unique_ptr<ASTNode> sExpr, const std::string& mName, int line)
        : ASTNode(NodeType::MemberAccessExpression, line),
          structExpr(std::move(sExpr)), memberName(mName) {}
    void print(int indent = 0) const override;
};

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
