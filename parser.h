#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <stack>
#include <unordered_map>
#include <algorithm>

#include "scanner.h"
#include "token.h"
#include "ast_nodes.h"
#include "symbol_table.h"

class Parser {
private:
    Scanner& scanner;
    Token currentToken;
    SymbolTable& symbolTable;

    void advance();
    void match(TokenType expectedType);
    void reportError(const std::string& message);
    const Token& peek(int k = 1);
    std::unordered_map<TokenType, int> operatorPrecedence;//哈希表，存放操作符的优先级

    int getPrecedence(TokenType opType);
    bool isTypeKeyword(TokenType type) const;
    bool isAssignmentOperator(TokenType type) const;

    std::unique_ptr<StatementListNode> parseStatementList();
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseBlockStatement();
    std::unique_ptr<DeclarationStatementNode> parseDeclarationStatement(bool isParam = false);

    std::unique_ptr<ASTNode> parseTypeSpecifier();
    std::unique_ptr<IfStatementNode> parseIfStatement();
    std::unique_ptr<WhileStatementNode> parseWhileStatement();
    std::unique_ptr<ForStatementNode> parseForStatement();
    std::unique_ptr<StructiDefinitionNode> parseStructiDefinitionStatement();
    std::unique_ptr<PrintStatementNode> parsePrintStatement();
    std::unique_ptr<FunctionDefinitionNode> parseFunctionDefinition();
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();
    std::unique_ptr<SwitchStatementNode> parseSwitchStatement();
    std::unique_ptr<BreakStatementNode> parseBreakStatement();
    std::unique_ptr<ContinueStatementNode> parseContinueStatement();

    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseAssignmentExpression();
    std::unique_ptr<ASTNode> parseBinaryExpressionRHS(int exprPrec, std::unique_ptr<ASTNode> lhs);
    std::unique_ptr<ASTNode> parseUnaryExpression();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parsePrimaryExpression();
    std::unique_ptr<ASTNode> parseInitializerList();

public:
    Parser(Scanner& s, SymbolTable& st);
    std::unique_ptr<ProgramNode> parse();
};

std::string tokenTypeToString(TokenType type);

#endif // PARSER_H
