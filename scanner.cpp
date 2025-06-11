#include "scanner.h"
#include <iostream>

// --- 构造与析构 ---
Scanner::Scanner(const std::string& filename)
    : currentLine(1), currentChar('\0'), currentColumn(0) {
    sourceFile.open(filename);
    if (!sourceFile.is_open()) {
        std::cerr << "错误: 无法打开源文件: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    getNextCharInternal(); // 读取第一个字符以初始化
}

Scanner::~Scanner() {
    if (sourceFile.is_open()) {
        sourceFile.close();
    }
}

// 核心公共接口
// 获取下一个Token (优先从缓冲区读取)
Token Scanner::getNextToken() {
    ensureLookahead(1); // 确保缓冲区中至少有一个Token
    Token token = lookaheadBuffer.front();
    lookaheadBuffer.pop_front();
    return token;
}

// 预读第k个Token
const Token& Scanner::peekToken(int k) {
    ensureLookahead(k); // 确保缓冲区中至少有k个Token
    return lookaheadBuffer[k - 1];
}

// 内部实现

// 确保缓冲区中至少有k个Token
void Scanner::ensureLookahead(int k) {
    while (lookaheadBuffer.size() < k) {
        lookaheadBuffer.push_back(fetchNextToken());
    }
}

// 真正从源文件扫描并构建下一个Token的函数
Token Scanner::fetchNextToken() {
    while (true) {
        skipWhitespace();
        if (currentChar == '/' && (sourceFile.peek() == '/' || sourceFile.peek() == '*')) {
            skipComment();
        } else {
            break;
        }
    }

    if (currentChar == EOF) {
        return Token(TokenType::END_OF_FILE, "EOF", currentLine);
    }

    if (isAlpha(currentChar)) {
        return processIdentifierOrKeyword();
    }
    if (isDigit(currentChar) || (currentChar == '.' && isDigit(sourceFile.peek()))) {
        return processNumber();
    }
    if (currentChar == '\'') {
        return processCharLiteral();
    }
    if (currentChar == '"') {
        return processStringLiteral();
    }

    return processOperatorOrDelimiter();
}


// --- 字符处理和跳过逻辑 ---
char Scanner::getNextCharInternal() {
    if (sourceFile.get(currentChar)) {
        if (currentChar == '\n') {
            currentLine++;
            currentColumn = 0;
        } else {
            currentColumn++;
        }
    } else {
        currentChar = EOF;
    }
    return currentChar;
}

bool Scanner::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void Scanner::skipWhitespace() {
    while (currentChar != EOF && isWhitespace(currentChar)) {
        getNextCharInternal();
    }
}

void Scanner::skipComment() {
    if (currentChar == '/') {
        if (sourceFile.peek() == '/') { // 单行注释
            while (currentChar != EOF && currentChar != '\n') {
                getNextCharInternal();
            }
        } else if (sourceFile.peek() == '*') { // 多行注释
            getNextCharInternal(); // 消耗 '/'
            getNextCharInternal(); // 消耗 '*'
            while (currentChar != EOF && !(currentChar == '*' && sourceFile.peek() == '/')) {
                getNextCharInternal();
            }
            if (currentChar != EOF) {
                getNextCharInternal(); // 消耗 '*'
                getNextCharInternal(); // 消耗 '/'
            }
        }
    }
}


// --- Token识别逻辑 ---
bool Scanner::isDigit(char c) const { return c >= '0' && c <= '9'; }
bool Scanner::isAlpha(char c) const { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Scanner::isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }

Token Scanner::processIdentifierOrKeyword() {
    std::string lexeme_str;
    int startLine = currentLine;
    while (currentChar != EOF && isAlphaNumeric(currentChar)) {
        lexeme_str += currentChar;
        getNextCharInternal();
    }
    if (keywordMap.count(lexeme_str)) {
        return Token(keywordMap.at(lexeme_str), lexeme_str, startLine);
    }
    return Token(TokenType::IDENTIFIER, lexeme_str, startLine);
}

Token Scanner::processNumber() {
    std::string lexeme_str;
    int startLine = currentLine;
    bool isFloat = false;

    // 解析整数部分
    while (currentChar != EOF && isDigit(currentChar)) {
        lexeme_str += currentChar;
        getNextCharInternal();
    }

    // 解析小数部分
    if (currentChar == '.') {
        isFloat = true;
        lexeme_str += currentChar;
        getNextCharInternal();
        while (currentChar != EOF && isDigit(currentChar)) {
            lexeme_str += currentChar;
            getNextCharInternal();
        }
    }

    // 解析科学计数法部分
    if (currentChar == 'e' || currentChar == 'E') {
        isFloat = true; // 包含科学计数法的数总是浮点数
        lexeme_str += currentChar;
        getNextCharInternal();

        // 处理可选的 '+' 或 '-'
        if (currentChar == '+' || currentChar == '-') {
            lexeme_str += currentChar;
            getNextCharInternal();
        }

        // 解析指数部分的数字
        if (!isDigit(currentChar)) {
            reportError("科学计数法中缺少指数部分");
        }
        while (currentChar != EOF && isDigit(currentChar)) {
            lexeme_str += currentChar;
            getNextCharInternal();
        }
    }

    if (isFloat) {
        return Token(TokenType::FLOAT_LITERAL, lexeme_str, startLine);
    }
    return Token(TokenType::INT_LITERAL, lexeme_str, startLine);
}

Token Scanner::processCharLiteral() {
    int startLine = currentLine;
    getNextCharInternal(); // 消耗起始 '
    char val = currentChar;
    getNextCharInternal(); // 消耗字符本身
    if (currentChar != '\'') {
        reportError("字符字面量未闭合");
    }
    getNextCharInternal(); // 消耗结束 '
    return Token(TokenType::CHAR_LITERAL, std::string(1, val), startLine);
}

Token Scanner::processStringLiteral() {
    std::string lexeme_str;
    int startLine = currentLine;
    getNextCharInternal(); // 消耗起始 "
    while (currentChar != EOF && currentChar != '"') {
        lexeme_str += currentChar;
        getNextCharInternal();
    }
    if (currentChar != '"') {
        reportError("字符串字面量未闭合");
    }
    getNextCharInternal(); // 消耗结束 "
    return Token(TokenType::STRING_LITERAL, lexeme_str, startLine);
}

Token Scanner::processOperatorOrDelimiter() {
    int startLine = currentLine;
    std::string op(1, currentChar);
    // 尝试匹配双字符操作符
    if (sourceFile.peek() != EOF) {
        std::string two_char_op = op + (char)sourceFile.peek();
        if (operatorMap.count(two_char_op)) {
            getNextCharInternal();
            getNextCharInternal();
            return Token(operatorMap.at(two_char_op), two_char_op, startLine);
        }
    }
    // 匹配单字符操作符
    if (operatorMap.count(op)) {
        getNextCharInternal();
        return Token(operatorMap.at(op), op, startLine);
    }

    reportError("未识别的符号: " + op);
    getNextCharInternal();
    return Token(TokenType::UNKNOWN, op, startLine);
}

void Scanner::reportError(const std::string& message) const {
    std::cerr << "词法错误 [行: " << currentLine << ", 列: " << currentColumn << "]: " << message << std::endl;
}
