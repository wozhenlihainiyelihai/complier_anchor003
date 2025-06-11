#include "parser.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

// 实现了完整的 tokenTypeToString 函数以提供更清晰的错误信息
string tokenTypeToString(TokenType type) {
    switch(type) {
        case TokenType::KW_ANCHOR: return "KW_ANCHOR";
        case TokenType::KW_MAIN: return "KW_MAIN";
        case TokenType::KW_INT: return "KW_INT";
        case TokenType::KW_FLOAT: return "KW_FLOAT";
        case TokenType::KW_CHAR: return "KW_CHAR";
        case TokenType::KW_BOOL: return "KW_BOOL";
        case TokenType::KW_STRING: return "KW_STRING";
        case TokenType::KW_VOID: return "KW_VOID";
        case TokenType::KW_CONST: return "KW_CONST";
        case TokenType::KW_LET: return "KW_LET";
        case TokenType::KW_AUTO: return "KW_AUTO";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELSE: return "KW_ELSE";
        case TokenType::KW_WHILE: return "KW_WHILE";
        case TokenType::KW_FOR: return "KW_FOR";
        case TokenType::KW_STRUCTI: return "KW_STRUCTI";
        case TokenType::KW_RETURN: return "KW_RETURN";
        case TokenType::KW_BREAK: return "KW_BREAK";
        case TokenType::KW_CONTINUE: return "KW_CONTINUE";
        case TokenType::KW_SWITCH: return "KW_SWITCH";
        case TokenType::KW_CASE: return "KW_CASE";
        case TokenType::KW_DEFAULT: return "KW_DEFAULT";
        case TokenType::KW_NEW: return "KW_NEW";
        case TokenType::KW_DELETE: return "KW_DELETE";
        case TokenType::KW_SIZEOF: return "KW_SIZEOF";
        case TokenType::KW_TRUE: return "KW_TRUE";
        case TokenType::KW_FALSE: return "KW_FALSE";
        case TokenType::KW_UNION: return "KW_UNION";
        case TokenType::KW_ENUM: return "KW_ENUM";
        case TokenType::KW_PRINT: return "KW_PRINT";
        case TokenType::KW_INPUT: return "KW_INPUT";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INT_LITERAL: return "INT_LITERAL";
        case TokenType::FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TokenType::CHAR_LITERAL: return "CHAR_LITERAL";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::MOD: return "MOD";
        case TokenType::PLUS_ASSIGN: return "PLUS_ASSIGN";
        case TokenType::MINUS_ASSIGN: return "MINUS_ASSIGN";
        case TokenType::STAR_ASSIGN: return "STAR_ASSIGN";
        case TokenType::SLASH_ASSIGN: return "SLASH_ASSIGN";
        case TokenType::MOD_ASSIGN: return "MOD_ASSIGN";
        case TokenType::EQ: return "EQ";
        case TokenType::NEQ: return "NEQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LE: return "LE";
        case TokenType::GE: return "GE";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::INC: return "INC";
        case TokenType::DEC: return "DEC";
        case TokenType::BIT_AND: return "BIT_AND";
        case TokenType::BIT_OR: return "BIT_OR";
        case TokenType::BIT_XOR: return "BIT_XOR";
        case TokenType::BIT_NOT: return "BIT_NOT";
        case TokenType::LSHIFT: return "LSHIFT";
        case TokenType::RSHIFT: return "RSHIFT";
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::COLON_OP: return "COLON_OP";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::DOT: return "DOT";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::UNKNOWN: return "UNKNOWN";
        default: return "UNHANDLED_TOKEN_TYPE (" + std::to_string(static_cast<int>(type)) + ")";
    }
}

Parser::Parser(Scanner& s, SymbolTable& st)
    : scanner(s), symbolTable(st), currentToken(scanner.getNextToken()) {
    operatorPrecedence[TokenType::ASSIGN] = 1;
    operatorPrecedence[TokenType::PLUS_ASSIGN] = 1;
    operatorPrecedence[TokenType::MINUS_ASSIGN] = 1;
    operatorPrecedence[TokenType::STAR_ASSIGN] = 1;
    operatorPrecedence[TokenType::SLASH_ASSIGN] = 1;
    operatorPrecedence[TokenType::MOD_ASSIGN] = 1;
    operatorPrecedence[TokenType::OR] = 2;
    operatorPrecedence[TokenType::AND] = 3;
    operatorPrecedence[TokenType::EQ] = 4;
    operatorPrecedence[TokenType::NEQ] = 4;
    operatorPrecedence[TokenType::LT] = 5;
    operatorPrecedence[TokenType::GT] = 5;
    operatorPrecedence[TokenType::LE] = 5;
    operatorPrecedence[TokenType::GE] = 5;
    operatorPrecedence[TokenType::PLUS] = 6;
    operatorPrecedence[TokenType::MINUS] = 6;
    operatorPrecedence[TokenType::STAR] = 7;
    operatorPrecedence[TokenType::SLASH] = 7;
    operatorPrecedence[TokenType::MOD] = 7;
}

void Parser::advance() { currentToken = scanner.getNextToken(); }

void Parser::match(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        advance();
    } else {
        reportError("期望的Token是 " + tokenTypeToString(expectedType) + ", 但实际得到的是 " + tokenTypeToString(currentToken.type) + " (词素: \"" + currentToken.lexeme + "\")");
    }
}

void Parser::reportError(const string& message) {
    cerr << "语法错误 在行 " << currentToken.line << ": " << message << endl;
    exit(EXIT_FAILURE);
}

const Token& Parser::peek(int k) {
    return scanner.peekToken(k);
}

int Parser::getPrecedence(TokenType opType) {
    if (operatorPrecedence.count(opType)) {
        return operatorPrecedence.at(opType);
    }
    return -1;
}

bool Parser::isTypeKeyword(TokenType type) const {
    return type == TokenType::KW_INT || type == TokenType::KW_FLOAT ||
           type == TokenType::KW_CHAR || type == TokenType::KW_BOOL ||
           type == TokenType::KW_STRING || type == TokenType::KW_VOID;
}

bool Parser::isAssignmentOperator(TokenType type) const {
    return type == TokenType::ASSIGN || type == TokenType::PLUS_ASSIGN ||
           type == TokenType::MINUS_ASSIGN || type == TokenType::STAR_ASSIGN ||
           type == TokenType::SLASH_ASSIGN || type == TokenType::MOD_ASSIGN;
}

unique_ptr<ProgramNode> Parser::parse() {
    int line = currentToken.line;
    auto stmts = make_unique<StatementListNode>(line);

    while (currentToken.type != TokenType::KW_ANCHOR && currentToken.type != TokenType::END_OF_FILE) {
        auto stmt = parseStatement();
        if (stmt) {
            stmts->addStatement(std::move(stmt));
        }
    }

    if (currentToken.type == TokenType::KW_ANCHOR) {
        match(TokenType::KW_ANCHOR);
        auto mainBlock = parseBlockStatement();

        if (mainBlock) {
             auto* mainStmts = static_cast<StatementListNode*>(mainBlock.get());
             for (auto& stmt : mainStmts->statements) {
                stmts->addStatement(std::move(stmt));
             }
             mainStmts->statements.clear();
        }
    }

    match(TokenType::END_OF_FILE);
    return make_unique<ProgramNode>(std::move(stmts), line);
}


unique_ptr<StatementListNode> Parser::parseStatementList() {
    int line = currentToken.line;
    auto stmtList = make_unique<StatementListNode>(line);
    while (currentToken.type != TokenType::RBRACE &&
           currentToken.type != TokenType::KW_CASE &&
           currentToken.type != TokenType::KW_DEFAULT &&
           currentToken.type != TokenType::END_OF_FILE) {
        auto stmt = parseStatement();
        if (stmt) {
            stmtList->addStatement(std::move(stmt));
        }
    }
    return stmtList;
}

unique_ptr<ASTNode> Parser::parseBlockStatement() {
    match(TokenType::LBRACE);
    auto block = parseStatementList();
    match(TokenType::RBRACE);
    return block;
}

unique_ptr<ASTNode> Parser::parseStatement() {
    switch (currentToken.type) {
        case TokenType::KW_IF: return parseIfStatement();
        case TokenType::KW_WHILE: return parseWhileStatement();
        case TokenType::KW_FOR: return parseForStatement();
        case TokenType::KW_STRUCTI: return parseStructiDefinitionStatement();
        case TokenType::KW_PRINT: {
            auto stmt = parsePrintStatement();
            match(TokenType::SEMICOLON);
            return stmt;
        }
        case TokenType::KW_RETURN: {
            auto stmt = parseReturnStatement();
            match(TokenType::SEMICOLON);
            return stmt;
        }
        case TokenType::LBRACE: return parseBlockStatement();
        case TokenType::SEMICOLON:
            advance();
            return nullptr;
        case TokenType::KW_SWITCH: return parseSwitchStatement();
        case TokenType::KW_BREAK: {
             auto stmt = parseBreakStatement();
             match(TokenType::SEMICOLON);
             return stmt;
        }
        case TokenType::KW_CONTINUE: {
             auto stmt = parseContinueStatement();
             match(TokenType::SEMICOLON);
             return stmt;
        }
        default:
            if (isTypeKeyword(currentToken.type) ||
               (currentToken.type == TokenType::IDENTIFIER &&
               (peek(1).type == TokenType::IDENTIFIER || peek(1).type == TokenType::LBRACKET)))
            {
                 int lookahead_count = 1;
                 while(peek(lookahead_count).type == TokenType::LBRACKET){
                    lookahead_count++;
                    if(peek(lookahead_count).type != TokenType::RBRACKET){
                        while(peek(lookahead_count).type != TokenType::RBRACKET  && peek(lookahead_count).type != TokenType::END_OF_FILE ) lookahead_count++;
                    }
                    if(peek(lookahead_count).type == TokenType::RBRACKET) lookahead_count++; else break;
                 }
                 if (peek(lookahead_count + 1).type == TokenType::LPAREN) {
                    return parseFunctionDefinition();
                 } else {
                    auto decl = parseDeclarationStatement();
                    match(TokenType::SEMICOLON);
                    return decl;
                 }
            } else {
                auto expr = parseExpression();
                match(TokenType::SEMICOLON);
                return expr;
            }
    }
}

unique_ptr<ASTNode> Parser::parseInitializerList() {
    int line = currentToken.line;
    match(TokenType::LBRACE);

    vector<unique_ptr<ASTNode>> elements;
    if (currentToken.type != TokenType::RBRACE) {
        while (true) {
            // 在这里，每个元素都可以是另一个表达式，其中可能包含另一个初始化列表
            elements.push_back(parseExpression());
            if (currentToken.type == TokenType::COMMA) {
                advance();
                if (currentToken.type == TokenType::RBRACE) { // 允许尾随逗号
                    break;
                }
            } else {
                break;
            }
        }
    }

    match(TokenType::RBRACE);
    return make_unique<InitializerListNode>(std::move(elements), line);
}

unique_ptr<DeclarationStatementNode> Parser::parseDeclarationStatement(bool isParam) {
    int line = currentToken.line;

    auto typeNode = parseTypeSpecifier();

    string idName = currentToken.lexeme;
    match(TokenType::IDENTIFIER);

    while (currentToken.type == TokenType::LBRACKET) {
        if(isParam) {
            reportError("函数参数不支持 C 风格的数组声明，请使用 `int[] a`。");
        }
        int arrayLine = currentToken.line;
        match(TokenType::LBRACKET);

        unique_ptr<ASTNode> sizeExpr = nullptr;
        if (currentToken.type != TokenType::RBRACKET) {
            sizeExpr = parseExpression();
        } else {
            if (peek(1).type != TokenType::ASSIGN || peek(2).type != TokenType::LBRACE) {
                reportError("C 风格的空括号 `[]` 数组声明必须带有初始化列表。");
            }
        }

        match(TokenType::RBRACKET);
        typeNode = make_unique<ArrayTypeNode>(std::move(typeNode), std::move(sizeExpr), arrayLine);
    }

    unique_ptr<ASTNode> initialValue = nullptr;
    if (currentToken.type == TokenType::ASSIGN) {
        if(isParam) reportError("函数参数不允许有默认值。");
        advance();
        if (currentToken.type == TokenType::LBRACE) {
            initialValue = parseInitializerList();
        } else {
            initialValue = parseExpression();
        }
    }

    return make_unique<DeclarationStatementNode>(std::move(typeNode), idName, std::move(initialValue), line);
}


unique_ptr<ASTNode> Parser::parseTypeSpecifier() {
    int line = currentToken.line;
    unique_ptr<ASTNode> type;

    if (isTypeKeyword(currentToken.type) || currentToken.type == TokenType::IDENTIFIER) {
        type = make_unique<TypeNode>(currentToken.lexeme, line);
        advance();
    } else {
        reportError("期望一个类型名，但得到 " + tokenTypeToString(currentToken.type));
        return nullptr;
    }

    while (currentToken.type == TokenType::LBRACKET && peek(1).type == TokenType::RBRACKET) {
        int arrayLine = currentToken.line;
        match(TokenType::LBRACKET);
        match(TokenType::RBRACKET);

        type = make_unique<ArrayTypeNode>(std::move(type), nullptr, arrayLine);
    }

    return type;
}


unique_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition() {
    int line = currentToken.line;
    auto returnType = parseTypeSpecifier();
    if (returnType->nodeType != ASTNode::NodeType::Type) {
        reportError("函数定义中的返回类型必须是基础类型。");
    }

    string funcName = currentToken.lexeme;
    match(TokenType::IDENTIFIER);
    match(TokenType::LPAREN);
    auto params = make_unique<StatementListNode>(currentToken.line);
    if (currentToken.type != TokenType::RPAREN) {
        while (true) {
            params->addStatement(parseDeclarationStatement(true));
            if (currentToken.type == TokenType::COMMA) {
                advance();
            } else {
                break;
            }
        }
    }
    match(TokenType::RPAREN);
    auto body = parseBlockStatement();
    auto bodyStmtList = unique_ptr<StatementListNode>(static_cast<StatementListNode*>(body.release()));

    auto returnTypeNode = unique_ptr<TypeNode>(static_cast<TypeNode*>(returnType.release()));

    return make_unique<FunctionDefinitionNode>(funcName, std::move(returnTypeNode), std::move(params), std::move(bodyStmtList), line);
}

unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    int line = currentToken.line;
    match(TokenType::KW_RETURN);
    unique_ptr<ASTNode> returnValue = nullptr;
    if (currentToken.type != TokenType::SEMICOLON) {
        returnValue = parseExpression();
    }
    return make_unique<ReturnStatementNode>(std::move(returnValue), line);
}

unique_ptr<IfStatementNode> Parser::parseIfStatement() {
    int line = currentToken.line;
    match(TokenType::KW_IF);
    match(TokenType::LPAREN);
    auto condition = parseExpression();
    match(TokenType::RPAREN);
    auto thenBlock = parseStatement();
    unique_ptr<ASTNode> elseBlock = nullptr;
    if (currentToken.type == TokenType::KW_ELSE) {
        advance();
        elseBlock = parseStatement();
    }
    return make_unique<IfStatementNode>(std::move(condition), std::move(thenBlock), std::move(elseBlock), line);
}


unique_ptr<WhileStatementNode> Parser::parseWhileStatement() {
    int line = currentToken.line;
    match(TokenType::KW_WHILE);
    match(TokenType::LPAREN);
    auto condition = parseExpression();
    match(TokenType::RPAREN);
    auto loopBlock = parseStatement();
    return make_unique<WhileStatementNode>(std::move(condition), std::move(loopBlock), line);
}

unique_ptr<ForStatementNode> Parser::parseForStatement() {
    int line = currentToken.line;
    match(TokenType::KW_FOR);
    match(TokenType::LPAREN);
    unique_ptr<ASTNode> initialization = nullptr;
    if (currentToken.type != TokenType::SEMICOLON) {
        if (isTypeKeyword(currentToken.type) || (currentToken.type == TokenType::IDENTIFIER && (peek(1).type == TokenType::IDENTIFIER || peek(1).type == TokenType::LBRACKET))) {
            initialization = parseDeclarationStatement();
        } else {
            initialization = parseExpression();
        }
    }
    match(TokenType::SEMICOLON);
    unique_ptr<ASTNode> condition = nullptr;
    if (currentToken.type != TokenType::SEMICOLON) {
        condition = parseExpression();
    }
    match(TokenType::SEMICOLON);
    unique_ptr<ASTNode> increment = nullptr;
    if (currentToken.type != TokenType::RPAREN) {
        increment = parseExpression();
    }
    match(TokenType::RPAREN);
    auto body = parseStatement();
    auto bodyStmtList = unique_ptr<StatementListNode>(static_cast<StatementListNode*>(body.release()));
    return make_unique<ForStatementNode>(std::move(initialization), std::move(condition), std::move(increment), std::move(bodyStmtList), line);
}

unique_ptr<PrintStatementNode> Parser::parsePrintStatement() {
    int line = currentToken.line;
    match(TokenType::KW_PRINT);
    match(TokenType::LPAREN);
    auto expr = parseExpression();
    match(TokenType::RPAREN);
    return make_unique<PrintStatementNode>(std::move(expr), line);
}

unique_ptr<StructiDefinitionNode> Parser::parseStructiDefinitionStatement() {
    int line = currentToken.line;
    match(TokenType::KW_STRUCTI);
    string structiName = currentToken.lexeme;
    match(TokenType::IDENTIFIER);
    match(TokenType::LBRACE);
    auto members = make_unique<StatementListNode>(currentToken.line);
    while (currentToken.type != TokenType::RBRACE) {
        auto memberDecl = parseDeclarationStatement(false);
        match(TokenType::SEMICOLON);
        members->addStatement(std::move(memberDecl));
    }
    match(TokenType::RBRACE);
    match(TokenType::SEMICOLON);
    return make_unique<StructiDefinitionNode>(structiName, std::move(members), line);
}

unique_ptr<SwitchStatementNode> Parser::parseSwitchStatement() {
    int line = currentToken.line;
    match(TokenType::KW_SWITCH);
    match(TokenType::LPAREN);
    auto expr = parseExpression();
    match(TokenType::RPAREN);
    match(TokenType::LBRACE);

    vector<unique_ptr<CaseStatementNode>> cases;
    while(currentToken.type == TokenType::KW_CASE || currentToken.type == TokenType::KW_DEFAULT) {
        int caseLine = currentToken.line;
        unique_ptr<ASTNode> value = nullptr;

        if (currentToken.type == TokenType::KW_CASE) {
            advance();
            value = parseExpression();
        } else {
            advance();
        }
        match(TokenType::COLON);

        auto body = parseStatementList();
        cases.push_back(make_unique<CaseStatementNode>(std::move(value), std::move(body), caseLine));
    }

    match(TokenType::RBRACE);
    return make_unique<SwitchStatementNode>(std::move(expr), std::move(cases), line);
}

unique_ptr<BreakStatementNode> Parser::parseBreakStatement() {
    int line = currentToken.line;
    match(TokenType::KW_BREAK);
    return make_unique<BreakStatementNode>(line);
}

unique_ptr<ContinueStatementNode> Parser::parseContinueStatement() {
    int line = currentToken.line;
    match(TokenType::KW_CONTINUE);
    return make_unique<ContinueStatementNode>(line);
}

unique_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (currentToken.type == TokenType::NOT || currentToken.type == TokenType::MINUS || currentToken.type == TokenType::INC || currentToken.type == TokenType::DEC) {
        Token opToken = currentToken;
        advance();
        auto operand = parseUnaryExpression();
        return make_unique<UnaryExpressionNode>(opToken.lexeme, std::move(operand), opToken.line);
    }
    return parseFactor();
}

unique_ptr<ASTNode> Parser::parseFactor() {
    auto node = parsePrimaryExpression();
    while (true) {
        if (currentToken.type == TokenType::LPAREN) {
            int line = currentToken.line;
            advance();
            vector<unique_ptr<ASTNode>> args;
            if (currentToken.type != TokenType::RPAREN) {
                while (true) {
                    args.push_back(parseExpression());
                    if (currentToken.type == TokenType::COMMA) {
                        advance();
                    } else {
                        break;
                    }
                }
            }
            match(TokenType::RPAREN);
            node = make_unique<FunctionCallNode>(std::move(node), std::move(args), line);
        } else if (currentToken.type == TokenType::LBRACKET) {
            int line = currentToken.line;
            advance();
            auto indexExpr = parseExpression();
            match(TokenType::RBRACKET);
            node = make_unique<ArrayAccessNode>(std::move(node), std::move(indexExpr), line);
        } else if (currentToken.type == TokenType::DOT) {
            int line = currentToken.line;
            advance();
            string memberName = currentToken.lexeme;
            match(TokenType::IDENTIFIER);
            node = make_unique<MemberAccessNode>(std::move(node), memberName, line);
        } else {
            break;
        }
    }
    return node;
}

// --- 修改: 恢复对 LBRACE 的处理以支持嵌套列表 ---
unique_ptr<ASTNode> Parser::parsePrimaryExpression() {
    int line = currentToken.line;
    unique_ptr<ASTNode> node;
    switch (currentToken.type) {
        case TokenType::IDENTIFIER:
            node = make_unique<IdentifierNode>(currentToken.lexeme, line);
            advance();
            break;
        case TokenType::INT_LITERAL:
        case TokenType::FLOAT_LITERAL:
        case TokenType::CHAR_LITERAL:
        case TokenType::STRING_LITERAL:
        case TokenType::KW_TRUE:
        case TokenType::KW_FALSE:
            node = make_unique<LiteralNode>(currentToken.lexeme, currentToken.type, line);
            advance();
            break;
        case TokenType::LPAREN:
            advance();
            node = parseExpression();
            match(TokenType::RPAREN);
            break;
        case TokenType::LBRACE: // 恢复此 case
            node = parseInitializerList();
            break;
        default:
            reportError("在表达式中期望标识符、字面量或括号，但得到 " + tokenTypeToString(currentToken.type));
    }
    return node;
}

unique_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignmentExpression();
}

unique_ptr<ASTNode> Parser::parseAssignmentExpression() {
    auto lhs = parseBinaryExpressionRHS(2, parseUnaryExpression());

    if (isAssignmentOperator(currentToken.type)) {
        Token opToken = currentToken;
        advance();
        auto rhs = parseAssignmentExpression();
        return make_unique<AssignmentStatementNode>(std::move(lhs), opToken.lexeme, std::move(rhs), opToken.line);
    }

    return lhs;
}

unique_ptr<ASTNode> Parser::parseBinaryExpressionRHS(int exprPrec, unique_ptr<ASTNode> lhs) {
    while (true) {
        int tokPrec = getPrecedence(currentToken.type);
        if (tokPrec < exprPrec) {
            return lhs;
        }
        Token opToken = currentToken;
        advance();
        auto rhs = parseUnaryExpression();
        int nextPrec = getPrecedence(currentToken.type);

        if (tokPrec < nextPrec) {
            rhs = parseBinaryExpressionRHS(tokPrec + 1, std::move(rhs));
        }
        lhs = make_unique<BinaryExpressionNode>(std::move(lhs), opToken.lexeme, std::move(rhs), opToken.line);
    }
}
