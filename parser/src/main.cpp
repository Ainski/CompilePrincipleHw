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

#include "../include/CLI/CLI.hpp"
#include "../include/TokenStream.h"
#include "../include/lexer.h"
#include "../include/logprintf.h"
#include "../include/parser.h"
#include "../include/SemanticAnalyzer.h"
#include "../include/IRGenerator.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

// ============================================================
//  main
// ============================================================
int main(int argc, char *argv[]) {

  CLI::App app{"Rust-like Language Parser + Semantic Analyzer + IR Generator"};

  bool quiet = false;
  app.add_flag("-q,--quiet", quiet,
               "Quiet mode: suppress banner and verbose output");

  string lexer_output = "";
  app.add_option("--lexer-output", lexer_output,
                 "Output file for lexer tokens (TSV format)");

  string parser_output = "parser_output.txt";
  app.add_option("--parser-output", parser_output,
                 "Output file for parser results");

  string semantic_output = "";
  app.add_option("--semantic-output", semantic_output,
                 "Output file for semantic analysis results");

  string ir_output = "";
  app.add_option("--ir-output", ir_output,
                 "Output file for intermediate code (quadruples)");

  bool print_tokens = false;
  app.add_flag("--print-tokens", print_tokens, "Print tokens to console");

  string input = "";
  app.add_option("--input", input, "Input file path (Original Rust File)");

#ifdef WINDOWS_BUILD
  bool use_gui = false;
  app.add_flag("--gui", use_gui, "Enable GUI interface");
#endif

  try {
    app.parse(argc, argv);
    if (!quiet)
      LOGO();
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
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

#ifdef WINDOWS_BUILD
  if (use_gui) {
    log(LogLevel::INFO, "Launching GUI mode...");
    extern void runGui(const std::string &input_file);
    runGui(input);
    return 0;
  }
#endif

  log(LogLevel::INFO, "Token stream loaded successfully from: " + input);

  TokenStream ts = lex(input);

  if (lexer_output != "") {
    ts.to_file(lexer_output);
  }
  if (print_tokens) {
    ts.displayTokens();
  }
  ts.remove_comments();

  Parser parser(ts);
  NodePtr tree;
  try {
    tree = parser.parse();
    if (print_tokens) {
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
  } catch (const exception &e) {
    cerr << "\n=== Parse error ===\n" << e.what() << "\n";
    return 1;
  }

  // ---- Semantic Analysis ----
  if (!quiet)
    log(LogLevel::INFO, "Running semantic analysis...");

  SemanticAnalyzer analyzer;
  analyzer.analyze(tree.get());

  if (analyzer.hasErrors()) {
    if (!quiet)
      cout << "\n=== Semantic Errors ===\n";
    if (semantic_output != "") {
      ofstream ofs(semantic_output);
      analyzer.printErrors(ofs);
      log(LogLevel::INFO, "Semantic errors written to: " + semantic_output);
      ofs.close();
    }
    analyzer.printErrors(cout);
    return 1;
  }

  if (!quiet)
    cout << "=== Semantic analysis passed ===\n" << endl;

  if (semantic_output != "") {
    ofstream ofs(semantic_output);
    ofs << "Semantic analysis passed. No errors found.\n";
    ofs.close();
    log(LogLevel::INFO, "Semantic analysis result written to: " + semantic_output);
  }

  // ---- IR Generation ----
  if (!quiet)
    log(LogLevel::INFO, "Generating intermediate code...");

  IRGenerator irgen;
  irgen.generate(tree.get());

  if (ir_output != "") {
    irgen.writeIR(ir_output);
    log(LogLevel::INFO, "IR written to: " + ir_output);
  } else {
    if (!quiet) {
      cout << "\n=== Intermediate Code (Quadruples) ===\n";
      irgen.printIR(cout);
      cout << "=== IR generation complete ===\n" << endl;
    }
  }

  return 0;
}
