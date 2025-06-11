#include "parser.h"

using namespace std;

//part one～
//初始化解析器，传入当前处理scanner
Parser::Parser(Scanner& s, SymbolTable& st)//用s来遍历，用st来检测
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
// 提供清晰的错误信息（抛出错误的时候使用，一个小工具）
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
        case TokenType::KW_NEW: return "KW_NEW";//最终未实现
        case TokenType::KW_DELETE: return "KW_DELETE";//最终未实现
        case TokenType::KW_SIZEOF: return "KW_SIZEOF";//最终未实现
        case TokenType::KW_TRUE: return "KW_TRUE";
        case TokenType::KW_FALSE: return "KW_FALSE";
        case TokenType::KW_UNION: return "KW_UNION";//最终未实现
        case TokenType::KW_ENUM: return "KW_ENUM";//最终未实现
        case TokenType::KW_PRINT: return "KW_PRINT";
        case TokenType::KW_INPUT: return "KW_INPUT";//最终未实现
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

//辅助函数：
//跳转到下一个语法单元
void Parser::advance() { currentToken = scanner.getNextToken(); }
//检查语法单元是否是期待款
void Parser::match(TokenType expectedType) {
    if (currentToken.type == expectedType) {
        advance();
    } else {
        reportError("期望的Token是 " + tokenTypeToString(expectedType) + ", 但实际得到的是 " + tokenTypeToString(currentToken.type) + " (词素: \"" + currentToken.lexeme + "\")");
    }
}
//报错并直接终止程序（其实不太合适，但是暂时先这样吧）
void Parser::reportError(const string& message) {
    cerr << "语法错误 在行 " << currentToken.line << ": " << message << endl;
    exit(EXIT_FAILURE);
}
//预读k个词法单元
const Token& Parser::peek(int k) {
    return scanner.peekToken(k);
}
//get到算符优先级
int Parser::getPrecedence(TokenType opType) {
    if (operatorPrecedence.count(opType)) { //检查是否存在（明显没有value为0的节点）
        return operatorPrecedence.at(opType); //返回value
    }
    return -1;//没找着
}
//is keyword ornot
bool Parser::isTypeKeyword(TokenType type) const {
    return type == TokenType::KW_INT || type == TokenType::KW_FLOAT ||
           type == TokenType::KW_CHAR || type == TokenType::KW_BOOL ||
           type == TokenType::KW_STRING || type == TokenType::KW_VOID;//only 6 keywo
}
//is 赋值运算符 ornot
bool Parser::isAssignmentOperator(TokenType type) const {
    return type == TokenType::ASSIGN || type == TokenType::PLUS_ASSIGN ||
           type == TokenType::MINUS_ASSIGN || type == TokenType::STAR_ASSIGN ||
           type == TokenType::SLASH_ASSIGN || type == TokenType::MOD_ASSIGN;
}

//part two～
//开始解析程序，直到文件结束，返回指向ast语法树的根节点指针
unique_ptr<ProgramNode> Parser::parse() {
    int line = currentToken.line;
    auto stmts = make_unique<StatementListNode>(line);//存储顶层语句

    //循环处理anchor和文件结尾之外的字符（全局变量，函数，结构体）
    while (currentToken.type != TokenType::KW_ANCHOR && currentToken.type != TokenType::END_OF_FILE) {
        auto stmt = parseStatement(); //解析单挑语句
        if (stmt) {
            stmts->addStatement(std::move(stmt)); //将语句节点添加到列表中，必须要用move，不然报错…………
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
             mainStmts->statements.clear();//清空原来的列表
        }
    }

    match(TokenType::END_OF_FILE);
    return make_unique<ProgramNode>(std::move(stmts), line);//返回根节点，其子节点是敖汉所有语句的列表stmts
}
//解析语句列表（一系列语句）
unique_ptr<StatementListNode> Parser::parseStatementList() {
    int line = currentToken.line;
    auto stmtList = make_unique<StatementListNode>(line);
    while (currentToken.type != TokenType::RBRACE &&
           currentToken.type != TokenType::KW_CASE &&
           currentToken.type != TokenType::KW_DEFAULT &&
           currentToken.type != TokenType::END_OF_FILE) {//如果尚未结束
        auto stmt = parseStatement();
        if (stmt) {
            stmtList->addStatement(std::move(stmt));
        }
    }
    return stmtList;
}
//解析由大括号包围的代码块
unique_ptr<ASTNode> Parser::parseBlockStatement() {
    match(TokenType::LBRACE);
    auto block = parseStatementList();
    match(TokenType::RBRACE);
    return block;
}
//解析单条语句
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
                 int lookahead_count = 1; //向前看多远，用来判断是函数定义还是变量声明
                 while(peek(lookahead_count).type == TokenType::LBRACKET){
                    lookahead_count++;
                    if(peek(lookahead_count).type != TokenType::RBRACKET){
                        while(peek(lookahead_count).type != TokenType::RBRACKET  && peek(lookahead_count).type != TokenType::END_OF_FILE ) lookahead_count++;
                    }
                    if(peek(lookahead_count).type == TokenType::RBRACKET) lookahead_count++; else break;
                 }
                 if (peek(lookahead_count + 1).type == TokenType::LPAREN) {
                    return parseFunctionDefinition(); //如果是括号，那么就是函数定义
                 } else {
                    auto decl = parseDeclarationStatement(); //否则就是一个变量声明
                    match(TokenType::SEMICOLON);
                    return decl;
                 }
            } else { //都不是，那就是表达式
                auto expr = parseExpression();
                match(TokenType::SEMICOLON);
                return expr;
            }
    }
}
//解析初始化列表{}
unique_ptr<ASTNode> Parser::parseInitializerList() {
    int line = currentToken.line;
    match(TokenType::LBRACE);

    vector<unique_ptr<ASTNode>> elements;
    if (currentToken.type != TokenType::RBRACE) {
        while (true) {
            // 在这里，每个元素都可以是另一个表达式，其中可能包含另一个初始化列表
            elements.push_back(parseExpression());
            if (currentToken.type == TokenType::COMMA) {//如果下一个是，
                advance();
                if (currentToken.type == TokenType::RBRACE) { // 允许尾随逗号，即允许{1,2,}
                    break;
                }
            } else { //不是逗号，那就直接结束
                break;
            }
        }
    }

    match(TokenType::RBRACE);
    return make_unique<InitializerListNode>(std::move(elements), line);
}
//解析一个变量声明语句 int a;/int function(){}/int a[]/int a=1
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
        if (currentToken.type != TokenType::RBRACKET) { //如果不空，
            sizeExpr = parseExpression();
        } else { //如果空，必须初始化
            if (peek(1).type != TokenType::ASSIGN || peek(2).type != TokenType::LBRACE) {
                reportError("C 风格的空括号 `[]` 数组声明必须带有初始化列表。");
            }
        }

        match(TokenType::RBRACKET);
        typeNode = make_unique<ArrayTypeNode>(std::move(typeNode), std::move(sizeExpr), arrayLine);
    }

    unique_ptr<ASTNode> initialValue = nullptr;
    if (currentToken.type == TokenType::ASSIGN) { //等号
        if(isParam) reportError("函数参数不允许有默认值。");
        advance();
        if (currentToken.type == TokenType::LBRACE) { //左大括号
            initialValue = parseInitializerList(); //初始化列表
        } else {
            initialValue = parseExpression(); //那就是表达式
        }
    }

    return make_unique<DeclarationStatementNode>(std::move(typeNode), idName, std::move(initialValue), line);
}
//解析类型说明符，比如数组/结构体
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
//解析函数定义
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
//解析return语句
unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    int line = currentToken.line;
    match(TokenType::KW_RETURN);
    unique_ptr<ASTNode> returnValue = nullptr;
    if (currentToken.type != TokenType::SEMICOLON) {
        returnValue = parseExpression();
    }
    return make_unique<ReturnStatementNode>(std::move(returnValue), line);
}
//ifelse
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
//while
unique_ptr<WhileStatementNode> Parser::parseWhileStatement() {
    int line = currentToken.line;
    match(TokenType::KW_WHILE);
    match(TokenType::LPAREN);
    auto condition = parseExpression();
    match(TokenType::RPAREN);
    auto loopBlock = parseStatement();
    return make_unique<WhileStatementNode>(std::move(condition), std::move(loopBlock), line);
}
//for
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
//print
unique_ptr<PrintStatementNode> Parser::parsePrintStatement() {
    int line = currentToken.line;
    match(TokenType::KW_PRINT);
    match(TokenType::LPAREN);
    auto expr = parseExpression();
    match(TokenType::RPAREN);
    return make_unique<PrintStatementNode>(std::move(expr), line);
}
//结构体定义
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
//switch
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
//break
unique_ptr<BreakStatementNode> Parser::parseBreakStatement() {
    int line = currentToken.line;
    match(TokenType::KW_BREAK);
    return make_unique<BreakStatementNode>(line);
}
//continue
unique_ptr<ContinueStatementNode> Parser::parseContinueStatement() {
    int line = currentToken.line;
    match(TokenType::KW_CONTINUE);
    return make_unique<ContinueStatementNode>(line);
}

//算法：算符优先
//解析表达式入口，调用优先级较低的赋值运算开始
unique_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignmentExpression();
}
//解析赋值表达式=
unique_ptr<ASTNode> Parser::parseAssignmentExpression() {
    //首先，尝试解析一个更高优先级的表达式作为作操作数，然后解析大于等于2优先级的其他运算
    auto lhs = parseBinaryExpressionRHS(2, parseUnaryExpression());

    if (isAssignmentOperator(currentToken.type)) {
        Token opToken = currentToken;
        advance();
        auto rhs = parseAssignmentExpression();
        return make_unique<AssignmentStatementNode>(std::move(lhs), opToken.lexeme, std::move(rhs), opToken.line);
    }

    return lhs;
}
//解析二元表达式（递归处理，算符优先）
unique_ptr<ASTNode> Parser::parseBinaryExpressionRHS(int exprPrec, unique_ptr<ASTNode> lhs) {
    while (true) {
        int tokPrec = getPrecedence(currentToken.type);
        if (tokPrec < exprPrec) {//如果优先级达不到要求
            return lhs; //返回
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
//解析一元表达式：取非，相反数
unique_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (currentToken.type == TokenType::NOT || currentToken.type == TokenType::MINUS || currentToken.type == TokenType::INC || currentToken.type == TokenType::DEC) {
        Token opToken = currentToken;
        advance();
        auto operand = parseUnaryExpression();
        return make_unique<UnaryExpressionNode>(opToken.lexeme, std::move(operand), opToken.line);
    }
    return parseFactor();
}
//解析因子：包含函数调用，数组访问，成员访问等后缀操作
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
//解析基本表达式单元：标识符，常量，括号等等
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