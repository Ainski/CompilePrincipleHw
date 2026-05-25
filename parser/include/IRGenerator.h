#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "parser.h"
#include "SemanticType.h"
#include "Symbol.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <sstream>

using namespace std;

enum class IROp {
    LABEL, ASSIGN, ADD, SUB, MUL, DIV,
    EQ, NE, LT, LE, GT, GE,
    JUMP, JZ, JNZ,
    PARAM, CALL, RETURN,
    INDEX_STORE, INDEX_LOAD,
    REF, DEREF,
    NEG, NOP,
    FUNC_BEGIN, FUNC_END,
    TUPLE_GET, ARRAY_LIT,
    BREAK, CONTINUE
};

string irOpToString(IROp op);

struct Quadruple {
    IROp op;
    string arg1;
    string arg2;
    string result;

    Quadruple(IROp o, string a1 = "", string a2 = "", string r = "")
        : op(o), arg1(move(a1)), arg2(move(a2)), result(move(r)) {}

    string toString() const;
};

class IRGenerator {
    vector<Quadruple> code;
    int temp_counter = 0;
    int label_counter = 0;
    int in_loop = 0;
    string break_label;
    string continue_label;

    unordered_map<string, shared_ptr<FunctionInfo>> functions;
    shared_ptr<FunctionInfo> current_function;

    string newTemp() { return "t" + to_string(temp_counter++); }
    string newLabel() { return "L" + to_string(label_counter++); }

    void emit(IROp op, const string& a1 = "", const string& a2 = "", const string& r = "") {
        code.emplace_back(op, a1, a2, r);
    }

    // Helpers
    string extractId(const Node* node) const;
    string extractLeafValue(const Node* node) const;
    bool hasLeafChild(const Node* node, const string& cat) const;
    STypePtr parseTypeFromNode(const Node* node);
    const Node* findChild(const Node* node, const string& label) const;
    vector<const Node*> findChildren(const Node* node, const string& label) const;

    // Generate IR for each construct
    void genProgram(const Node* node);
    void genFunction(const Node* node);
    void genBlock(const Node* node);
    void genStmt(const Node* node);
    void genLetStmt(const Node* node);
    void genAssignStmt(const Node* node);
    void genReturnStmt(const Node* node);
    void genIfStmt(const Node* node);
    void genWhileStmt(const Node* node);
    void genForStmt(const Node* node);
    void genLoopStmt(const Node* node);
    void genBreakStmt(const Node* node);
    void genContinueStmt(const Node* node);
    void genExprStmt(const Node* node);

    // Returns the temp/variable holding the expression result
    string genExpr(const Node* node);
    string genLiteral(const Node* node);
    string genAtom(const Node* node);
    string genCallExpr(const Node* node);
    string genIndexExpr(const Node* node);
    string genArrayLit(const Node* node);
    string genRangeExpr(const Node* node);
    string genRefExpr(const Node* node);
    string genDerefExpr(const Node* node);
    string genBinary(const Node* node, bool isCmp);

public:
    void generate(const Node* root);

    const vector<Quadruple>& getIR() const { return code; }
    void printIR(ostream& os = cout) const;
    void writeIR(const string& filename) const;
};

#endif
