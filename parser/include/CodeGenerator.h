#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "IR.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

using namespace std;

// ============================================================
//  CodeGenerator —— 四元式 IR → x86-64 AT&T 汇编（作业3 M3，A 档）
//
//  策略（先正确后优化）：
//  - 每函数固定 1024 字节栈帧；变量/临时变量按需分配 -8/-16/... 栈槽
//  - 运算经 %rax/%rcx；System V AMD64 调用约定（参数 rdi/rsi/.../r9，返回 rax）
//  - 生成 _start 入口调用 --entry 指定函数，exit code = 返回值（便于运行验证）
// ============================================================
class CodeGenerator {
    const vector<Quadruple>& code_;
    ostringstream out_;
    unordered_map<string, int> frame_;     // name → 栈偏移（正，用 -N(%rbp) 访问）
    unordered_set<string> array_names_;    // 数组变量名（分配 80 字节连续栈区，10 元素）
    int next_offset_ = 0;
    int func_param_idx_ = 0;               // 被调用者 ASSIGN param 用
    int call_param_idx_ = 0;               // 调用者 PARAM 用
    string func_end_;                      // 当前函数结束标签
    string entry_;                         // _start 调用的入口函数

    bool isNum(const string& s) const;
    int slot(const string& name);          // 分配/取栈槽
    string operand(const string& s);       // → $imm | -N(%rbp) | %reg(param_)
    void emit(const string& s) { out_ << "    " << s << "\n"; }
    void emitLabel(const string& s) { out_ << s << "\n"; }

public:
    CodeGenerator(const vector<Quadruple>& ir, const string& entry = "")
        : code_(ir), entry_(entry) {}

    string generate();
    void writeAsm(const string& filename) const;
};

#endif
