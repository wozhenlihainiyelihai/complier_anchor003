#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <limits>
#include <filesystem>

// 包含所有编译器组件
#include "token.h"
#include "symbol_table.h"
#include "scanner.h"
#include "ast_nodes.h"
#include "parser.h"
#include "quadruple.h"
#include "ir_generator.h"
#include "optimizer.h"
#include "code_generator.h"
#include "tinyfiledialogs.h"
using namespace std;

#define MY_SOURCE_NAME "test_ir_correct.anchor"

int main(int argc, char* argv[]) {
    initializeKeywordMap();
    initializeOperatorMap();

    std::cout << "Anchor 编译器 - 目标代码生成模式" << std::endl;
    std::cout << "======================================" << std::endl;

    // 1. 获取源文件
    std::string sourceFilename;
    // 提供交互式选择
    std::cout << "请选择输入方式:\n";
    std::cout << "1. 使用默认测试文件\n";
    std::cout << "2. 浏览并选择文件 (图形界面)\n";
    std::cout << "请选择(1/2): ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除输入缓冲区

    if (choice == 1) {
        sourceFilename = MY_SOURCE_NAME;
        std::cout << "使用默认测试文件: " << sourceFilename << std::endl;
    } else if (choice == 2) {
        // 使用图形界面文件选择对话框
        const char* filterPatterns[1] = {"*.anchor"}; // 文件过滤器
        const char* selected = tinyfd_openFileDialog(
        "选择 Anchor 源文件", // 对话框标题
        "",                   // 初始路径 (空表示默认)
        1,                    // 过滤器数量
        filterPatterns,        // 文件类型过滤器
        "Anchor 源文件 (*.anchor)", // 过滤器描述
        0                      // 允许多选? 0=否
        );
        if (selected) {
            sourceFilename = selected;
            std::cout << "选择的文件: " << sourceFilename << std::endl;
        } else {
            std::cout << "未选择文件，将使用默认测试文件\n";
            sourceFilename = MY_SOURCE_NAME;
        }
    } else{
        std::cout << "无效选择，将使用默认测试文件\n";
        sourceFilename = MY_SOURCE_NAME;
    }
    std::cout << "--------------------------------------" << std::endl;


    // 2. 词法分析
    std::cout << "\n[阶段 1.5: 词法分析测试]" << std::endl;
    std::cout << "--- 扫描到的 Tokens ---" << std::endl;
    Scanner testScanner(sourceFilename);
    int tokenCount = 0;
    while (true) {
        Token token = testScanner.getNextToken();
        // 对输出进行格式化，使其更易读
        std::cout << "Token #" << ++tokenCount << "\t"
                  << "行: " << token.line << ",\t"
                  << "类型: " << tokenTypeToString(token.type) << ",\t"
                  << "词素: '" << token.lexeme << "'" << std::endl;
        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
    }
    std::cout << "--- 词法分析测试结束 ---" << std::endl;


    // 3. 语法分析
    Scanner mainScanner(sourceFilename);
    SymbolTable symbolTable;
    Parser parser(mainScanner, symbolTable);
    std::unique_ptr<ProgramNode> astRoot = parser.parse();

    if (!astRoot) {
        std::cerr << "语法分析失败, 终止编译。" << std::endl;
        return 1;
    }
    std::cout << "\n[阶段 2: 语法分析] - AST 生成成功" << std::endl;
    astRoot->print(0);


    // 4. 语义分析与IR生成
    IRGenerator irGenerator(std::move(astRoot), symbolTable);
    irGenerator.generate();
    const auto& quadruples = irGenerator.getQuadruples();
    std::cout << "\n[阶段 3: 中间代码生成] - 原始四元式" << std::endl;
    irGenerator.dumpQuadruples();


    // 5. 中间代码优化阶段
    std::cout << "\n[阶段 4: 中间代码优化]" << std::endl;
    Optimizer optimizer(quadruples, symbolTable);
    std::vector<Quadruple> optimizedQuads = optimizer.optimize();

    std::cout << "--- 优化后的四元式 ---" << std::endl;
    for (size_t i = 0; i < optimizedQuads.size(); ++i) {
        cout << i << ":\t" << optimizedQuads[i].toString() << endl;
    }
    cout << "--- 四元式结束 ---" << endl;

    // 目标代码生成阶段
    std::cout << "\n[阶段 5: 目标代码生成]" << std::endl;
    // 使用优化后的四元式序列！
    CodeGenerator codeGen(optimizedQuads, symbolTable);
    std::string assemblyCode = codeGen.generate();

    std::cout << "--- 生成的 x86 汇编代码 ---" << std::endl;
    //std::cout << assemblyCode << std::endl;
    std::cout << "--- 汇编代码结束 ---" << std::endl;

    std::ofstream outFile("output.s");
    if(outFile.is_open()){
        outFile << assemblyCode;
        outFile.close();
        std::cout << "汇编代码已保存到 output.s 文件中。" << std::endl;
    }

    std::cout << "\nAnchor 编译器所有阶段执行完毕。" << std::endl;
    return 0;
}
