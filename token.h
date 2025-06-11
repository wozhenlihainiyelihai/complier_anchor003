#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <unordered_map>

// 定义Token的类型枚举
enum class TokenType {
    // 程序结构关键字
    KW_ANCHOR,      // "anchor" 关键字
    KW_MAIN,        // "main" 关键字

    // 类型关键字
    KW_INT,         // "int" 类型关键字
    KW_FLOAT,       // "float" 类型关键字
    KW_CHAR,        // "char" 类型关键字
    KW_BOOL,        // "bool" 类型关键字
    KW_STRING,      // "string" 类型关键字
    KW_VOID,        // "void" 类型关键字
    KW_CONST,       // "const" 修饰符关键字
    KW_LET,         // "let" 声明关键字
    KW_AUTO,        // "auto" 类型推断关键字

    // 控制结构关键字
    KW_IF,          // "if" 条件语句关键字
    KW_ELSE,        // "else" 条件语句关键字
    KW_WHILE,       // "while" 循环语句关键字
    KW_FOR,         // "for" 循环语句关键字
    KW_RETURN,      // "return" 返回语句关键字
    KW_BREAK,       // "break" 跳出循环关键字
    KW_CONTINUE,    // "continue" 继续循环关键字
    KW_SWITCH,      // "switch" 分支语句关键字
    KW_CASE,        // "case" 分支语句关键字
    KW_DEFAULT,     // "default" 分支语句关键字

    // 内存管理关键字
    KW_NEW,         // "new" 关键字
    KW_DELETE,      // "delete" 关键字
    KW_SIZEOF,      // "sizeof" 运算符关键字

    // 布尔字面量
    KW_TRUE,        // "true" 布尔真值
    KW_FALSE,       // "false" 布尔假值

    // 用户定义结构关键字
    KW_STRUCTI,      // "struct" 结构体关键字
    KW_UNION,       // "union" 联合体关键字
    KW_ENUM,        // "enum" 枚举关键字

    // 输入输出关键字
    KW_PRINT,       // "print" 输出函数关键字
    KW_INPUT,       // "input" 输入函数关键字 (如果支持)

    // 标识符 (变量名、函数名等)
    IDENTIFIER,

    // 字面量 (常量)
    INT_LITERAL,    // 整型字面量, 例如: 123, 0, -10
    FLOAT_LITERAL,  // 浮点型字面量, 例如: 3.14, -0.5, 1.0, 2. (根据语言定义)
    CHAR_LITERAL,   // 字符型字面量, 例如: 'a', '\n'
    STRING_LITERAL, // 字符串字面量, 例如: "hello", "你好\n"

    // 运算符
    ASSIGN,         // 赋值: =
    PLUS,           // 加: +
    MINUS,          // 减: -
    STAR,           // 乘: *
    SLASH,          // 除: /
    MOD,            // 取模: %

    PLUS_ASSIGN,    // 加等于: +=
    MINUS_ASSIGN,   // 减等于: -=
    STAR_ASSIGN,    // 乘等于: *=
    SLASH_ASSIGN,   // 除等于: /=
    MOD_ASSIGN,     // 模等于: %=

    EQ,             // 等于比较: ==
    NEQ,            // 不等于比较: !=
    LT,             // 小于: <
    GT,             // 大于: >
    LE,             // 小于等于: <=
    GE,             // 大于等于: >=

    AND,            // 逻辑与: &&
    OR,             // 逻辑或: ||
    NOT,            // 逻辑非: !

    INC,            // 自增: ++
    DEC,            // 自减: --

    // 位运算符
    BIT_AND,        // 按位与: &
    BIT_OR,         // 按位或: |
    BIT_XOR,        // 按位异或: ^
    BIT_NOT,        // 按位取反: ~
    LSHIFT,         // 左移: <<
    RSHIFT,         // 右移: >>

    // 三元运算符
    QUESTION,       // 问号: ?
    COLON_OP,       // 三元运算中的冒号: : (区别于普通冒号界符)

    // 界符 (分隔符和括号)
    SEMICOLON,      // 分号: ;
    COLON,          // 冒号: : (例如用于类型声明或标签)
    COMMA,          // 逗号: ,
    LPAREN,         // 左圆括号: (
    RPAREN,         // 右圆括号: )
    LBRACE,         // 左大括号: {
    RBRACE,         // 右大括号: }
    LBRACKET,       // 左方括号:[
    RBRACKET,       // 右方括号:]
    DOT,            // 点运算符

    // 特殊Token
    END_OF_FILE,    // 文件结束标记
    UNKNOWN         // 未识别的Token
};

// Token结构体定义, 用于表示词法分析器识别出的一个词法单元
struct Token {
    TokenType type;     // Token的类型 (来自TokenType枚举)
    std::string lexeme; // Token的词素 (原始文本值)
    int line;           // Token在源代码中的起始行号

    // 构造函数
    Token(TokenType t, const std::string& lex, int ln)
        : type(t), lexeme(lex), line(ln) {}

    // 默认构造函数, 初始化为一个无效/未知Token
    Token() : type(TokenType::UNKNOWN), lexeme(""), line(0) {}
};

// 外部声明Anchor语言的关键字映射表 (词素 -> TokenType)
extern std::unordered_map<std::string, TokenType> keywordMap;
// 初始化Anchor语言关键字映射表的函数声明
void initializeKeywordMap();

// 外部声明Anchor语言的运算符/界符映射表 (词素 -> TokenType)
extern std::unordered_map<std::string, TokenType> operatorMap;
// 初始化Anchor语言运算符/界符映射表的函数声明
void initializeOperatorMap();
#endif // TOKEN_H
