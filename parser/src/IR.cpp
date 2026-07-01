#include "../include/IR.h"

string irOpToString(IROp op) {
    switch (op) {
        case IROp::LABEL:       return "LABEL";
        case IROp::ASSIGN:      return "ASSIGN";
        case IROp::ADD:         return "ADD";
        case IROp::SUB:         return "SUB";
        case IROp::MUL:         return "MUL";
        case IROp::DIV:         return "DIV";
        case IROp::EQ:          return "EQ";
        case IROp::NE:          return "NE";
        case IROp::LT:          return "LT";
        case IROp::LE:          return "LE";
        case IROp::GT:          return "GT";
        case IROp::GE:          return "GE";
        case IROp::JUMP:        return "JUMP";
        case IROp::JZ:          return "JZ";
        case IROp::JNZ:         return "JNZ";
        case IROp::PARAM:       return "PARAM";
        case IROp::CALL:        return "CALL";
        case IROp::RETURN:      return "RETURN";
        case IROp::INDEX_STORE: return "INDEX_STORE";
        case IROp::INDEX_LOAD:  return "INDEX_LOAD";
        case IROp::REF:         return "REF";
        case IROp::DEREF:       return "DEREF";
        case IROp::NEG:         return "NEG";
        case IROp::NOP:         return "NOP";
        case IROp::FUNC_BEGIN:  return "FUNC_BEGIN";
        case IROp::FUNC_END:    return "FUNC_END";
        case IROp::TUPLE_GET:   return "TUPLE_GET";
        case IROp::ARRAY_LIT:   return "ARRAY_LIT";
        case IROp::BREAK:       return "BREAK";
        case IROp::CONTINUE:    return "CONTINUE";
    }
    return "?";
}

string Quadruple::toString() const {
    string s = "(" + irOpToString(op);
    s += ", " + (arg1.empty() ? "-" : arg1);
    s += ", " + (arg2.empty() ? "-" : arg2);
    s += ", " + (result.empty() ? "-" : result);
    return s + ")";
}
