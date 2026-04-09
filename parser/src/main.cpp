// ============================================================
//  Recursive Descent Parser for Rust-like language
//  Input : TSV token stream produced by the flex lexer
//          (stdin or first command-line argument)
//  Output: Indented parse tree  →  stdout
//          Error messages        →  stderr
//
//  Build : g++ -std=c++17 -o parser parser.cpp
//  Run   : ./parser  ../flex/output.tsv
//          ./flex/clex < ./flex/input.rs | ./parser/parser
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include "../include/TokenStream.h"
#include "../include/lexer.h"
#include "../include/CLI/CLI.hpp"
#include "../include/logprintf.h"
#include "../include/parser.h"

using namespace std;




// ============================================================
//  main
// ============================================================
int main(int argc, char* argv[]) {
    // 直接输出 ASCII 艺术字 LOGO
    LOGO();
    
    CLI::App app {"Rust-like Language Parser"};

    /**
     * 词法分析器输出文件
     */
    string lexer_output = "";
    app.add_option("--lexer-output",lexer_output, "Output file for lexer tokens (TSV format)");

    /**
     * 语法分析器输出文件
     */
    string parser_output = "parser_output.txt";
    app.add_option("--parser-output",parser_output, "Output file for parser results");  

    /**
     * 是否需要屏幕打印
     */
    bool print_tokens = false;
    app.add_flag("--print-tokens", print_tokens, "Print tokens to console");

    /**
     * 输入文件路径
     */

    string input = "";
    app.add_option("--input", input, "Input file path (Original Rust File)");


    /**
     * 是否启用gui界面
     */
#ifdef WINDOWS_BUILD
    bool use_gui = false;
    app.add_flag("--gui", use_gui, "Enable GUI interface");
#endif


    try{
        app.parse(argc, argv);
        if (input == "") {
            #ifdef WINDOWS_BUILD
                if (!use_gui) {
                    log(LogLevel::INFO, "you need to specify input file path");
                    return 1;
                }
            #else
                log(LogLevel::INFO, "you need to specify input file path");
                return 1;
            #endif
        }
    }catch(const CLI::ParseError&e){
        return app.exit(e);
    }

    // GUI 模式：如果启用则启动 GUI（不依赖任何 main 中的变量）
#ifdef WINDOWS_BUILD
    if(use_gui){
        log(LogLevel::INFO, "Launching GUI mode...");
        extern void runGui(const std::string& input_file);
        runGui(input);  // 只传递输入文件路径，GUI 内部完成所有分析
        return 0;
    }
#endif

    log(LogLevel::INFO, "Token stream loaded successfully from: " + input);

    TokenStream ts = lex(input); // 完成词法分析


    if(lexer_output != ""){

        ts.to_file(lexer_output);
    }
    if(print_tokens){
        ts.displayTokens();
    }
    ts.remove_comments();

    Parser parser(ts);
    try {
        NodePtr tree = parser.parse();
        if(print_tokens){
            log(LogLevel::INFO, "Parse tree:");
            tree->PrintToScreem();
            cout << "\n=== Parse successful ===\n" << endl;
        }

        if (parser_output != "") {
            ofstream ofs(parser_output);
            if (!ofs) {
                cerr << "Error: Cannot open output file: " << parser_output << "\n";
                return 1;
            }
            tree->PrintToFile(ofs);
            log(LogLevel::INFO, "Parse tree written to: " + parser_output);
            ofs.close();
        }
    } catch (const exception& e) {
        cerr << "\n=== Parse error ===\n" << e.what() << "\n";
        return 1;
    }


    

    return 0;
}
