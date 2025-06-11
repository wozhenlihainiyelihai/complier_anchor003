#include "token.h"

// 定义Anchor语言的关键字映射表
std::unordered_map<std::string, TokenType> keywordMap;
// 定义Anchor语言的运算符/界符映射表
std::unordered_map<std::string, TokenType> operatorMap;

// 初始化Anchor语言的关键字
void initializeKeywordMap() {
    // 清空以防重复初始化 (虽然通常只调用一次)
    keywordMap.clear();

    // 程序结构
    keywordMap["anchor"] = TokenType::KW_ANCHOR;
    keywordMap["main"] = TokenType::KW_MAIN;

    // 类型
    keywordMap["int"] = TokenType::KW_INT;
    keywordMap["float"] = TokenType::KW_FLOAT;
    keywordMap["char"] = TokenType::KW_CHAR;
    keywordMap["bool"] = TokenType::KW_BOOL;
    keywordMap["string"] = TokenType::KW_STRING;
    keywordMap["void"] = TokenType::KW_VOID;
    keywordMap["const"] = TokenType::KW_CONST;
    keywordMap["auto"] = TokenType::KW_AUTO;

    // 控制结构
    keywordMap["if"] = TokenType::KW_IF;
    keywordMap["else"] = TokenType::KW_ELSE;
    keywordMap["while"] = TokenType::KW_WHILE;
    keywordMap["for"] = TokenType::KW_FOR;
    keywordMap["return"] = TokenType::KW_RETURN; // 确保 'return' 关键字被识别
    keywordMap["break"] = TokenType::KW_BREAK;
    keywordMap["continue"] = TokenType::KW_CONTINUE;
    keywordMap["switch"] = TokenType::KW_SWITCH;
    keywordMap["case"] = TokenType::KW_CASE;
    keywordMap["default"] = TokenType::KW_DEFAULT;

    // 内存
    keywordMap["new"] = TokenType::KW_NEW;
    keywordMap["delete"] = TokenType::KW_DELETE;
    keywordMap["sizeof"] = TokenType::KW_SIZEOF;

    // 布尔字面量
    keywordMap["true"] = TokenType::KW_TRUE;
    keywordMap["false"] = TokenType::KW_FALSE;

    // 结构体等
    keywordMap["structi"] = TokenType::KW_STRUCTI;
    keywordMap["union"] = TokenType::KW_UNION;
    keywordMap["enum"] = TokenType::KW_ENUM;

    // IO
    keywordMap["print"] = TokenType::KW_PRINT;
    keywordMap["input"] = TokenType::KW_INPUT;
}

// 初始化Anchor语言的运算符和界符
void initializeOperatorMap() {
    // 清空以防重复初始化
    operatorMap.clear();

    // 赋值
    operatorMap["="] = TokenType::ASSIGN;
    operatorMap["+="] = TokenType::PLUS_ASSIGN;
    operatorMap["-="] = TokenType::MINUS_ASSIGN;
    operatorMap["*="] = TokenType::STAR_ASSIGN;
    operatorMap["/="] = TokenType::SLASH_ASSIGN;
    operatorMap["%="] = TokenType::MOD_ASSIGN;

    // 算术
    operatorMap["+"] = TokenType::PLUS;
    operatorMap["-"] = TokenType::MINUS;
    operatorMap["*"] = TokenType::STAR;
    operatorMap["/"] = TokenType::SLASH;
    operatorMap["%"] = TokenType::MOD;

    // 比较
    operatorMap["=="] = TokenType::EQ;
    operatorMap["!="] = TokenType::NEQ;
    operatorMap["<"] = TokenType::LT;
    operatorMap[">"] = TokenType::GT;
    operatorMap["<="] = TokenType::LE;
    operatorMap[">="] = TokenType::GE;

    // 逻辑
    operatorMap["&&"] = TokenType::AND;
    operatorMap["||"] = TokenType::OR;
    operatorMap["!"] = TokenType::NOT;

    // 自增/自减
    operatorMap["++"] = TokenType::INC;
    operatorMap["--"] = TokenType::DEC;

    // 界符
    operatorMap[";"] = TokenType::SEMICOLON;
    operatorMap[","] = TokenType::COMMA;
    operatorMap[":"] = TokenType::COLON;
    operatorMap["("] = TokenType::LPAREN;
    operatorMap[")"] = TokenType::RPAREN;
    operatorMap["{"] = TokenType::LBRACE;
    operatorMap["}"] = TokenType::RBRACE;
    operatorMap["["] = TokenType::LBRACKET;
    operatorMap["]"]=TokenType::RBRACKET;
    operatorMap["."] = TokenType::DOT;
}