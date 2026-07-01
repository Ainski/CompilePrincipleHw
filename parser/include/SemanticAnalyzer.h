#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "parser.h"
#include "Symbol.h"
#include "IR.h"   // IROp, Quadruple（作业3 a' 嫁接合并：语义分析同时生成 IR）
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
    bool block_tail_as_return = false;   // 7.2：函数体块末尾 TailExpr 作为隐式 return
    string loop_result;                  // 7.4：循环表达式 break 值的汇合临时变量
    unordered_map<const Node*, STypePtr> node_block_type;  // 7.x：块表达式的值类型

    // ---- IR 生成基础设施（作业3 a' 嫁接合并）----
    vector<Quadruple> code;
    int temp_counter = 0;
    int label_counter = 0;
    string break_label;
    string continue_label;
    unordered_map<const Node*, string> node_ir_temp;  // 表达式节点的求值结果（临时变量/字面量/变量名）
    bool emit_enabled = true;   // false 时 emit 不产出（抑制赋值左值的无谓求值，对齐 IRGenerator 行为）

    string newTemp() {
        // 抑制 emit（赋值左值求值）时不占用编号——对齐 IRGenerator 不对 lhs 求值
        if (!emit_enabled) return "";
        return "t" + to_string(temp_counter++);
    }
    string newLabel() { return "L" + to_string(label_counter++); }
    void emit(IROp op, const string& a1 = "", const string& a2 = "", const string& r = "") {
        if (!emit_enabled) return;
        code.emplace_back(op, a1, a2, r);
    }

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
    STypePtr visitIfExpr(const Node* node);      // 7.3 选择表达式
    STypePtr visitLoopExpr(const Node* node);    // 7.4 循环表达式

    void checkLvalue(const Node* node, int line);

public:
    void analyze(const Node* root);

    bool hasErrors() const { return !errors.empty(); }
    const vector<SemanticError>& getErrors() const { return errors; }
    void printErrors(ostream& os = cout) const;

    // IR 输出（作业3 a'）
    const vector<Quadruple>& getIR() const { return code; }
    void printIR(ostream& os = cout) const;
    void writeIR(const string& filename) const;
};

#endif
