#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <fstream>
#include <vector>
#include <deque> // 使用 deque 作为缓冲区，便于在前端增删
#include "token.h"

class Scanner {
private:
    std::ifstream sourceFile;
    char currentChar;
    int currentLine;
    int currentColumn;

    // 使用双端队列作为预读缓冲区
    std::deque<Token> lookaheadBuffer;

    // 核心词法分析逻辑 (现在是私有的)
    Token fetchNextToken();

    // 内部辅助函数
    char getNextCharInternal();
    bool isWhitespace(char c) const;
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    void skipWhitespace();
    void skipComment();
    Token processNumber();
    Token processCharLiteral();
    Token processStringLiteral();
    Token processIdentifierOrKeyword();
    Token processOperatorOrDelimiter();

    // 填充缓冲区的方法
    void ensureLookahead(int k);

public:
    Scanner(const std::string& filename);
    ~Scanner();

    // 公共接口
    Token getNextToken(); // 从缓冲区获取或直接扫描新Token
    const Token& peekToken(int k = 1); // 预读第k个Token

    void reportError(const std::string& message) const;
    int getCurrentLine() const { return currentLine; }
};

#endif //SCANNER_H
