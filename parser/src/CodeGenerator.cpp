#include "../include/CodeGenerator.h"
#include <fstream>
#include <iostream>
#include <cctype>

static const char* argRegs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

bool CodeGenerator::isNum(const string& s) const {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '-') { if (s.size() == 1) return false; i = 1; }
    for (; i < s.size(); i++) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

int CodeGenerator::slot(const string& name) {
    if (name.empty() || isNum(name)) return 0;
    auto it = frame_.find(name);
    if (it != frame_.end()) return it->second;
    int sz = array_names_.count(name) ? 80 : 8;   // 数组 80 字节（10 元素×8），其余 8
    next_offset_ += sz;
    frame_[name] = next_offset_;
    return next_offset_;
}

string CodeGenerator::operand(const string& s) {
    if (s.empty()) return "$0";
    if (isNum(s)) return "$" + s;
    if (s.rfind("param_", 0) == 0)
        return "%" + string(argRegs[func_param_idx_ % 6]);
    return "-" + to_string(slot(s)) + "(%rbp)";
}

string CodeGenerator::generate() {
    out_.str("");
    // 预扫描：收集数组变量名（ARRAY_LIT 的 result、INDEX_LOAD 的 arr、INDEX_STORE 的 arr）
    for (const auto& q : code_) {
        if (q.op == IROp::ARRAY_LIT && !q.result.empty()) array_names_.insert(q.result);
        if (q.op == IROp::INDEX_LOAD && !q.arg1.empty()) array_names_.insert(q.arg1);
        if (q.op == IROp::INDEX_STORE && !q.result.empty()) array_names_.insert(q.result);
        if (q.op == IROp::TUPLE_GET && !q.arg1.empty()) array_names_.insert(q.arg1);   // 元组也按连续区
    }
    out_ << "    .text\n";
    bool first = true;

    for (size_t i = 0; i < code_.size(); i++) {
        const Quadruple& q = code_[i];
        switch (q.op) {
        case IROp::FUNC_BEGIN:
            frame_.clear(); next_offset_ = 0; func_param_idx_ = 0;
            func_end_ = q.arg1 + "_end";
            if (first) { out_ << "    .globl " << q.arg1 << "\n"; first = false; }
            out_ << q.arg1 << ":\n";
            emit("push %rbp");
            emit("mov %rsp, %rbp");
            emit("sub $1024, %rsp");
            break;

        case IROp::FUNC_END:
            out_ << "." << func_end_ << ":\n";
            emit("mov %rbp, %rsp");
            emit("pop %rbp");
            emit("ret");
            break;

        case IROp::ASSIGN: {
            if (q.arg1.rfind("param_", 0) == 0) {
                emit("mov %" + string(argRegs[func_param_idx_ % 6]) + ", %rax");
                if (!q.result.empty()) emit("mov %rax, " + operand(q.result));
                func_param_idx_++;
            } else {
                emit("mov " + operand(q.arg1) + ", %rax");
                if (!q.result.empty()) emit("mov %rax, " + operand(q.result));
            }
            break;
        }

        case IROp::ADD: case IROp::SUB: case IROp::MUL: case IROp::DIV: {
            const char* ins = (q.op == IROp::ADD) ? "add"
                            : (q.op == IROp::SUB) ? "sub"
                            : (q.op == IROp::MUL) ? "imul" : "idiv";
            emit("mov " + operand(q.arg1) + ", %rax");
            emit("mov " + operand(q.arg2) + ", %rcx");
            if (q.op == IROp::DIV) { emit("cqo"); emit(string(ins) + " %rcx"); }
            else emit(string(ins) + " %rcx, %rax");
            emit("mov %rax, " + operand(q.result));
            break;
        }

        case IROp::EQ: case IROp::NE: case IROp::LT: case IROp::LE:
        case IROp::GT: case IROp::GE: {
            const char* setcc = (q.op == IROp::EQ) ? "sete"
                              : (q.op == IROp::NE) ? "setne"
                              : (q.op == IROp::LT) ? "setl"
                              : (q.op == IROp::LE) ? "setle"
                              : (q.op == IROp::GT) ? "setg" : "setge";
            emit("mov " + operand(q.arg1) + ", %rax");
            emit("cmp " + operand(q.arg2) + ", %rax");
            emit(string(setcc) + " %al");
            emit("movzbl %al, %eax");
            emit("mov %rax, " + operand(q.result));
            break;
        }

        case IROp::LABEL:
            out_ << "." << q.arg1 << ":\n";
            break;

        case IROp::JUMP:
            emit("jmp ." + q.arg1);
            break;

        case IROp::JZ:
            emit("mov " + operand(q.arg1) + ", %rax");
            emit("cmp $0, %rax");
            emit("je ." + q.arg2);
            break;

        case IROp::JNZ:
            emit("mov " + operand(q.arg1) + ", %rax");
            emit("cmp $0, %rax");
            emit("jne ." + q.arg2);
            break;

        case IROp::PARAM:
            emit("mov " + operand(q.arg1) + ", %" + string(argRegs[call_param_idx_ % 6]));
            call_param_idx_++;
            break;

        case IROp::CALL:
            emit("call " + q.arg1);
            call_param_idx_ = 0;
            if (!q.result.empty()) emit("mov %rax, " + operand(q.result));
            break;

        case IROp::RETURN:
            if (!q.arg1.empty()) emit("mov " + operand(q.arg1) + ", %rax");
            emit("jmp ." + func_end_);
            break;

        // 数组/引用（3.6）：栈上连续区，元素 i 在 base - i*8
        case IROp::ARRAY_LIT: {
            // elem(arg1), count(arg2), result(数组名)
            emit("mov " + operand(q.arg1) + ", %rax");
            int s = slot(q.result);
            int cnt = atoi(q.arg2.c_str());
            emit("mov %rax, -" + to_string(s + cnt * 8) + "(%rbp)");
            break;
        }
        case IROp::INDEX_LOAD: {
            // arr(arg1), idx(arg2), result
            int s = slot(q.arg1);
            emit("mov " + operand(q.arg2) + ", %rcx");
            emit("imul $8, %rcx");
            emit("lea -" + to_string(s) + "(%rbp), %rax");
            emit("sub %rcx, %rax");
            emit("mov (%rax), %rax");
            emit("mov %rax, " + operand(q.result));
            break;
        }
        case IROp::INDEX_STORE: {
            // val(arg1), idx(arg2), arr(result)
            int s = slot(q.result);
            emit("mov " + operand(q.arg1) + ", %rax");
            emit("mov " + operand(q.arg2) + ", %rcx");
            emit("imul $8, %rcx");
            emit("lea -" + to_string(s) + "(%rbp), %rdx");
            emit("sub %rcx, %rdx");
            emit("mov %rax, (%rdx)");
            break;
        }
        case IROp::REF: {
            // inner(arg1) 的地址 → result
            int s = slot(q.arg1);
            emit("lea -" + to_string(s) + "(%rbp), %rax");
            emit("mov %rax, " + operand(q.result));
            break;
        }
        case IROp::DEREF: {
            // inner(arg1) 是引用（存地址），解引用 → result
            if (q.arg2.empty()) {
                // DEREF inner, _, result（读取）：*(inner) → result
                emit("mov " + operand(q.arg1) + ", %rax");
                emit("mov (%rax), %rax");
                emit("mov %rax, " + operand(q.result));
            } else {
                // DEREF val, _, ptr（写入赋值左值）：*ptr = val
                emit("mov " + operand(q.arg1) + ", %rcx");
                emit("mov " + operand(q.arg2) + ", %rax");
                emit("mov %rcx, (%rax)");
            }
            break;
        }
        case IROp::TUPLE_GET: {
            // tuple(arg1), idx(arg2 字面量), result —— 元组元素 i 在 base - i*8
            int s = slot(q.arg1);
            int idx = atoi(q.arg2.c_str());
            emit("lea -" + to_string(s) + "(%rbp), %rax");
            emit("mov $" + to_string(idx * 8) + ", %rcx");
            emit("sub %rcx, %rax");
            emit("mov (%rax), %rax");
            emit("mov %rax, " + operand(q.result));
            break;
        }

        case IROp::NEG:
            emit("mov " + operand(q.arg1) + ", %rax");
            emit("neg %rax");
            emit("mov %rax, " + operand(q.result));
            break;

        default:
            out_ << "    # " << q.toString() << "\n";
            break;
        }
    }

    // _start 入口：调用 entry_ 函数（参数 0），exit code = 返回值
    if (!entry_.empty()) {
        out_ << "    .globl _start\n";
        out_ << "_start:\n";
        emit("mov $" + to_string(entry_arg_) + ", %rdi");  // 入口函数参数
        emit("call " + entry_);
        emit("mov %rax, %rdi");        // exit code = 返回值
        emit("mov $60, %rax");         // sys_exit
        emit("syscall");
    }

    return out_.str();
}

void CodeGenerator::writeAsm(const string& filename) const {
    ofstream ofs(filename);
    if (!ofs.is_open()) {
        cerr << "Error: Cannot open asm output file: " << filename << "\n";
        return;
    }
    ofs << out_.str();
    ofs.close();
}
