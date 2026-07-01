#ifndef IR_H
#define IR_H

#include <vector>
#include <string>
#include <iostream>

using namespace std;

// ============================================================
//  中间代码（四元式）基础类型 —— 作业3 a' 从 IRGenerator 抽出
//  SemanticAnalyzer 嫁接 IR 生成后直接使用，IRGenerator 已废弃
// ============================================================

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

#endif
