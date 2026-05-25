#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "parser.h"
#include "Symbol.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

using namespace std;

struct SemanticError {
    string message;
    int line;

    SemanticError(string msg, int ln) : message(move(msg)), line(ln) {}
};

class SemanticAnalyzer {
    SymbolTable symtab;
    unordered_map<string, shared_ptr<FunctionInfo>> functions;
    vector<SemanticError> errors;
    shared_ptr<FunctionInfo> current_function;
    int in_loop = 0;

    void error(const string& msg, int line);

    // helpers
    STypePtr parseTypeFromNode(const Node* node);
    shared_ptr<Symbol> lookupVar(const string& name, int line);
    string extractId(const Node* node) const;
    int extractLine(const Node* node) const;
    string extractLeafValue(const Node* node) const;
    int extractInt(const Node* node) const;

    // visitor methods — return expression type
    void visitProgram(const Node* node);
    void registerFunction(const Node* node);
    void visitFunction(const Node* node);
    void visitFuncHeader(const Node* node);
    void visitBlock(const Node* node);
    void visitStmt(const Node* node);
    void visitLetStmt(const Node* node);
    void visitAssignStmt(const Node* node);
    void visitReturnStmt(const Node* node);
    void visitIfStmt(const Node* node);
    void visitWhileStmt(const Node* node);
    void visitForStmt(const Node* node);
    void visitLoopStmt(const Node* node);
    void visitBreakStmt(const Node* node);
    void visitContinueStmt(const Node* node);
    void visitExprStmt(const Node* node);

    STypePtr visitExpr(const Node* node);
    STypePtr visitCmpExpr(const Node* node);
    STypePtr visitAddExpr(const Node* node);
    STypePtr visitMulExpr(const Node* node);
    STypePtr visitUnary(const Node* node);
    STypePtr visitRefExpr(const Node* node);
    STypePtr visitDerefExpr(const Node* node);
    STypePtr visitAtom(const Node* node);
    STypePtr visitLiteral(const Node* node);
    STypePtr visitCallExpr(const Node* node);
    STypePtr visitIndexExpr(const Node* node);
    STypePtr visitArrayLit(const Node* node);
    STypePtr visitTupleLit(const Node* node);
    STypePtr visitRangeExpr(const Node* node);
    STypePtr visitParenExpr(const Node* node);

    void checkLvalue(const Node* node, int line);

public:
    void analyze(const Node* root);

    bool hasErrors() const { return !errors.empty(); }
    const vector<SemanticError>& getErrors() const { return errors; }
    void printErrors(ostream& os = cout) const;
};

#endif
