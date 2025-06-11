#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include <vector>
#include <string>
#include <memory>
#include <map>

#include "ast_nodes.h"
#include "symbol_table.h"
#include "quadruple.h"

// 表达式求值结果的结构体
struct ExpressionResult {
    std::string place;
    std::shared_ptr<TypeInfo> type;
    bool isLValue;

    ExpressionResult(std::string p = "", std::shared_ptr<TypeInfo> t = nullptr, bool lval = false)
        : place(std::move(p)), type(std::move(t)), isLValue(lval) {}

    bool isValid() const {
        return type != nullptr && type->kind != TypeKind::UNKNOWN;
    }
};

class IRGenerator {
private:
    std::vector<Quadruple> quadruples;
    SymbolTable& symbolTable;
    std::unique_ptr<ASTNode> astRoot;
    std::shared_ptr<TypeInfo> currentFunctionReturnType;
    std::vector<std::string> breakLabels;
    std::vector<std::string> continueLabels;

    // 核心遍历方法
    void generate(ASTNode* node);
    ExpressionResult generateExpression(ASTNode* node, bool needsLValue = false);

    // 辅助函数，用于从AST类型节点获取符号表类型信息
    std::shared_ptr<TypeInfo> getTypeFromNode(ASTNode* typeNode);

    // 语义分析辅助函数
    void reportSemanticError(int line, const std::string& message);
    std::shared_ptr<TypeInfo> getExpressionType(ASTNode* node);
    bool checkAssignmentCompatibility(const std::shared_ptr<TypeInfo>& target, const std::shared_ptr<TypeInfo>& source, int line);
    std::shared_ptr<TypeInfo> checkOperationType(const std::shared_ptr<TypeInfo>& type1, const std::shared_ptr<TypeInfo>& type2, const std::string& op, int line);

    // 各AST节点的生成函数
    void generateStatementList(StatementListNode* node);

    std::string recursivelyInitializeArray(const std::string& nameHint,const std::shared_ptr<TypeInfo>& type, InitializerListNode* initList);

    void generateDeclarationStatement(DeclarationStatementNode* node);
    void generateAssignmentStatement(AssignmentStatementNode* node);
    void generateIfStatement(IfStatementNode* node);
    void generateWhileStatement(WhileStatementNode* node);
    void generateForStatement(ForStatementNode* node);
    void generatePrintStatement(PrintStatementNode* node);
    void generateStructiDefinition(StructiDefinitionNode* node);
    void generateFunctionDefinition(FunctionDefinitionNode* node);
    void generateReturnStatement(ReturnStatementNode* node);
    void generateSwitchStatement(SwitchStatementNode* node);
    void generateBreakStatement(BreakStatementNode* node);
    void generateContinueStatement(ContinueStatementNode* node);


    // 表达式的生成函数
    ExpressionResult generateFunctionCall(FunctionCallNode* node);
    ExpressionResult generateArrayAccess(ArrayAccessNode* node, bool needsLValue);
    ExpressionResult generateBinaryExpression(BinaryExpressionNode* node);
    ExpressionResult generateUnaryExpression(UnaryExpressionNode* node);
    ExpressionResult generateIdentifier(IdentifierNode* node, bool needsLValue);
    ExpressionResult generateLiteral(LiteralNode* node);
    ExpressionResult generateMemberAccess(MemberAccessNode* node, bool needsLValue);

public:
    IRGenerator(std::unique_ptr<ASTNode> root, SymbolTable& st);
    void generate();
    const std::vector<Quadruple>& getQuadruples() const { return quadruples; }
    void dumpQuadruples() const;
};

#endif // IR_GENERATOR_H
