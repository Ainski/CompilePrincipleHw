#include "../include/IRGenerator.h"

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
        case IROp::INDEX_LOAD:  return "INDEX_LOAD";
        case IROp::INDEX_STORE: return "INDEX_STORE";
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

// ============================================================
//  Helpers (same pattern as SemanticAnalyzer)
// ============================================================

string IRGenerator::extractId(const Node* node) const {
    if (!node) return "";
    if (node->isLeaf) {
        auto pos = node->label.find(": ");
        return pos != string::npos ? node->label.substr(pos + 2) : node->label;
    }
    for (auto& c : node->children) {
        string id = extractId(c.get());
        if (!id.empty()) return id;
    }
    return "";
}

string IRGenerator::extractLeafValue(const Node* node) const {
    if (!node) return "";
    if (node->isLeaf) {
        auto pos = node->label.find(": ");
        return pos != string::npos ? node->label.substr(pos + 2) : node->label;
    }
    return "";
}

bool isIRLeafCat(const Node* node, const string& cat) {
    if (!node || !node->isLeaf) return false;
    auto pos = node->label.find(": ");
    return pos != string::npos ? node->label.substr(0, pos) == cat : node->label == cat;
}

bool IRGenerator::hasLeafChild(const Node* node, const string& cat) const {
    for (auto& c : node->children)
        if (isIRLeafCat(c.get(), cat)) return true;
    return false;
}

const Node* IRGenerator::findChild(const Node* node, const string& label) const {
    for (auto& c : node->children)
        if (c->label == label) return c.get();
    return nullptr;
}

vector<const Node*> IRGenerator::findChildren(const Node* node, const string& label) const {
    vector<const Node*> r;
    for (auto& c : node->children)
        if (c->label == label) r.push_back(c.get());
    return r;
}

STypePtr IRGenerator::parseTypeFromNode(const Node* node) {
    if (!node || node->label != "Type") return SType::makeUnknown();
    auto& ch = node->children;
    if (ch.empty()) return SType::makeUnknown();
    auto& first = ch[0];
    if (isIRLeafCat(first.get(), "I32")) return SType::makeI32();
    if (isIRLeafCat(first.get(), "Identifier")) return SType::makeI32();
    if (isIRLeafCat(first.get(), "LBracket")) {
        auto elem = parseTypeFromNode(ch[1].get());
        int sz = stoi(extractLeafValue(ch[3].get()));
        return SType::makeArray(elem, sz);
    }
    if (isIRLeafCat(first.get(), "Ampersand")) {
        int idx = 1;
        bool mut = (ch.size() > 2 && isIRLeafCat(ch[1].get(), "Mut"));
        if (mut) idx = 2;
        auto inner = parseTypeFromNode(ch[idx].get());
        return mut ? SType::makeMutRef(inner) : SType::makeRef(inner);
    }
    if (isIRLeafCat(first.get(), "LParen")) {
        if (ch.size() <= 2 || isIRLeafCat(ch[1].get(), "RParen"))
            return SType::makeUnit();
        vector<STypePtr> types;
        for (size_t i = 1; i < ch.size(); i++)
            if (ch[i]->label == "Type") types.push_back(parseTypeFromNode(ch[i].get()));
        return types.empty() ? SType::makeUnit() : SType::makeTuple(move(types));
    }
    return SType::makeUnknown();
}

// ============================================================
//  Top-level generation
// ============================================================

void IRGenerator::generate(const Node* root) {
    if (root && root->label == "Program") genProgram(root);
}

void IRGenerator::genProgram(const Node* node) {
    for (auto& c : node->children)
        if (c->label == "Function") genFunction(c.get());
}

void IRGenerator::genFunction(const Node* node) {
    auto* header = findChild(node, "Function") ? findChild(node, "Function") :
                   (node->children.size() > 0 ? node->children[0].get() : nullptr);

    // Find FuncHeader
    auto* hdr = findChild(node, "FuncHeader");
    if (!hdr) return;

    auto& hc = hdr->children;
    string funcName = extractLeafValue(hc[1].get());

    // Parse function info
    vector<pair<string, STypePtr>> params;
    STypePtr retType = SType::makeVoid();

    auto* paramList = findChild(hdr, "ParamList");
    if (paramList) {
        auto paramNodes = findChildren(paramList, "Param");
        for (auto* p : paramNodes) {
            bool is_mut = hasLeafChild(p, "Mut");
            string pname = extractLeafValue(p->children[is_mut ? 1 : 0].get());
            auto* typeNode = findChild(p, "Type");
            auto ptype = typeNode ? parseTypeFromNode(typeNode) : SType::makeUnknown();
            params.emplace_back(pname, ptype);
        }
    }

    for (size_t i = 0; i < hc.size(); i++) {
        if (isIRLeafCat(hc[i].get(), "Arrow") && i + 1 < hc.size()) {
            retType = parseTypeFromNode(hc[i + 1].get());
            break;
        }
    }

    auto funcInfo = make_shared<FunctionInfo>(funcName, params, retType);
    functions[funcName] = funcInfo;
    current_function = funcInfo;

    // Emit function begin
    emit(IROp::FUNC_BEGIN, funcName);

    // Emit param assignments
    for (auto& [pname, ptype] : params) {
        emit(IROp::ASSIGN, "param_" + pname, "", pname);
    }

    // Generate function body
    auto* block = findChild(node, "Block");
    if (block) genBlock(block);

    // Emit function end
    string retLabel = "func_" + funcName + "_end";
    emit(IROp::LABEL, retLabel);
    emit(IROp::FUNC_END, funcName);

    current_function = nullptr;
}

void IRGenerator::genBlock(const Node* node) {
    for (auto& c : node->children) {
        if (c->isLeaf) continue;
        genStmt(c.get());
    }
}

// ============================================================
//  Statement generators
// ============================================================

void IRGenerator::genStmt(const Node* node) {
    const string& lbl = node->label;
    if (lbl == "EmptyStmt") {}
    else if (lbl == "LetStmt")     genLetStmt(node);
    else if (lbl == "AssignStmt")  genAssignStmt(node);
    else if (lbl == "ReturnStmt")  genReturnStmt(node);
    else if (lbl == "IfStmt")      genIfStmt(node);
    else if (lbl == "WhileStmt")   genWhileStmt(node);
    else if (lbl == "ForStmt")     genForStmt(node);
    else if (lbl == "LoopStmt")    genLoopStmt(node);
    else if (lbl == "BreakStmt")   genBreakStmt(node);
    else if (lbl == "ContinueStmt")genContinueStmt(node);
    else if (lbl == "ExprStmt")    genExprStmt(node);
    else if (lbl == "Block")       genBlock(node);
}

void IRGenerator::genLetStmt(const Node* node) {
    auto* varDecl = findChild(node, "VarDecl");
    if (!varDecl) return;

    bool is_mut = hasLeafChild(varDecl, "Mut");
    int idIdx = is_mut ? 1 : 0;
    string varName = extractLeafValue(varDecl->children[idIdx].get());

    // Check for initializer
    if (hasLeafChild(node, "Assign")) {
        // Find expression child
        for (auto& c : node->children) {
            if (!c->isLeaf && c->label != "VarDecl" && c->label != "LetStmt") {
                string result = genExpr(c.get());
                if (result != varName)
                    emit(IROp::ASSIGN, result, "", varName);
                break;
            }
        }
    }
}

void IRGenerator::genAssignStmt(const Node* node) {
    auto& ch = node->children;
    // Find Assign leaf
    int assignIdx = -1;
    for (size_t i = 0; i < ch.size(); i++) {
        if (isIRLeafCat(ch[i].get(), "Assign")) { assignIdx = (int)i; break; }
    }
    if (assignIdx < 0) return;

    auto* lhsNode = ch[0].get();
    string rhs = genExpr(ch[assignIdx + 1].get());

    if (lhsNode->label == "Identifier") {
        string name = extractId(lhsNode);
        if (rhs != name) emit(IROp::ASSIGN, rhs, "", name);
    } else if (lhsNode->label == "IndexExpr") {
        string arrName = extractId(lhsNode);
        // Find index expression
        for (auto& c : lhsNode->children) {
            if (!c->isLeaf && c->label != "IndexExpr") {
                string idx = genExpr(c.get());
                emit(IROp::INDEX_STORE, rhs, idx, arrName);
                break;
            }
        }
    } else if (lhsNode->label == "DerefExpr") {
        string ptr = genExpr(lhsNode->children[1].get());
        emit(IROp::DEREF, rhs, "", ptr);
    }
}

void IRGenerator::genReturnStmt(const Node* node) {
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "ReturnStmt") {
            string val = genExpr(c.get());
            emit(IROp::RETURN, val);
            return;
        }
    }
    emit(IROp::RETURN);
    if (current_function) {
        emit(IROp::JUMP, "func_" + current_function->name + "_end");
    }
}

void IRGenerator::genIfStmt(const Node* node) {
    // IfStmt: If, Expr, Block, [ElseClause]
    string elseLabel = newLabel();
    string endLabel = newLabel();

    // Condition
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "IfStmt" && c->label != "Block" && c->label != "ElseClause") {
            string cond = genExpr(c.get());
            emit(IROp::JZ, cond, elseLabel);
            break;
        }
    }

    // Then block
    auto blocks = findChildren(node, "Block");
    if (!blocks.empty()) genBlock(blocks[0]);

    // Jump over else
    emit(IROp::JUMP, endLabel);

    // Else label
    emit(IROp::LABEL, elseLabel);

    // Else clause
    auto* elseClause = findChild(node, "ElseClause");
    if (elseClause) {
        for (auto& c : elseClause->children) {
            if (!c->isLeaf) {
                if (c->label == "Block") genBlock(c.get());
                else if (c->label == "IfStmt") genIfStmt(c.get());
            }
        }
    }

    emit(IROp::LABEL, endLabel);
}

void IRGenerator::genWhileStmt(const Node* node) {
    string startLabel = newLabel();
    string endLabel = newLabel();

    emit(IROp::LABEL, startLabel);

    // Condition
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "WhileStmt" && c->label != "Block") {
            string cond = genExpr(c.get());
            emit(IROp::JZ, cond, endLabel);
            break;
        }
    }

    // Body
    string oldBreak = break_label;
    string oldContinue = continue_label;
    break_label = endLabel;
    continue_label = startLabel;

    auto* block = findChild(node, "Block");
    if (block) { in_loop++; genBlock(block); in_loop--; }

    emit(IROp::JUMP, startLabel);
    emit(IROp::LABEL, endLabel);

    break_label = oldBreak;
    continue_label = oldContinue;
}

void IRGenerator::genForStmt(const Node* node) {
    string startLabel = newLabel();
    string endLabel = newLabel();
    string stepLabel = newLabel();

    string varName;
    for (auto& c : node->children) {
        if (isIRLeafCat(c.get(), "Identifier")) {
            varName = extractLeafValue(c.get());
            break;
        }
    }

    emit(IROp::LABEL, startLabel);

    // Range expression - evaluate start and end
    for (auto& c : node->children) {
        if (c->label == "RangeExpr") {
            // RangeExpr: left, .., right
            string start = genExpr(c->children[0].get());
            string end = genExpr(c->children[2].get());

            string condTemp = newTemp();
            emit(IROp::LT, varName, end, condTemp);
            emit(IROp::JZ, condTemp, endLabel);
            break;
        }
    }

    string oldBreak = break_label;
    string oldContinue = continue_label;
    break_label = endLabel;
    continue_label = stepLabel;

    auto* block = findChild(node, "Block");
    if (block) { in_loop++; genBlock(block); in_loop--; }

    emit(IROp::LABEL, stepLabel);
    emit(IROp::ADD, varName, "1", varName);
    emit(IROp::JUMP, startLabel);
    emit(IROp::LABEL, endLabel);

    break_label = oldBreak;
    continue_label = oldContinue;
}

void IRGenerator::genLoopStmt(const Node* node) {
    string startLabel = newLabel();
    string endLabel = newLabel();

    emit(IROp::LABEL, startLabel);

    string oldBreak = break_label;
    string oldContinue = continue_label;
    break_label = endLabel;
    continue_label = startLabel;

    auto* block = findChild(node, "Block");
    if (block) { in_loop++; genBlock(block); in_loop--; }

    emit(IROp::JUMP, startLabel);
    emit(IROp::LABEL, endLabel);

    break_label = oldBreak;
    continue_label = oldContinue;
}

void IRGenerator::genBreakStmt(const Node* node) {
    if (!break_label.empty()) {
        emit(IROp::JUMP, break_label);
    }
}

void IRGenerator::genContinueStmt(const Node* node) {
    if (!continue_label.empty()) {
        emit(IROp::JUMP, continue_label);
    }
}

void IRGenerator::genExprStmt(const Node* node) {
    for (auto& c : node->children) {
        if (!c->isLeaf) { genExpr(c.get()); break; }
    }
}

// ============================================================
//  Expression generators
// ============================================================

string IRGenerator::genExpr(const Node* node) {
    if (!node) return "";
    const string& lbl = node->label;

    if (lbl == "Literal")    return genLiteral(node);
    if (lbl == "Identifier") return genAtom(node);
    if (lbl == "CallExpr")   return genCallExpr(node);
    if (lbl == "IndexExpr")  return genIndexExpr(node);
    if (lbl == "ArrayLit")   return genArrayLit(node);
    if (lbl == "TupleLit")   return genArrayLit(node);
    if (lbl == "ParenExpr") {
        for (auto& c : node->children)
            if (!c->isLeaf) return genExpr(c.get());
        return "";
    }
    if (lbl == "RangeExpr")  return genRangeExpr(node);
    if (lbl == "RefExpr")    return genRefExpr(node);
    if (lbl == "DerefExpr")  return genDerefExpr(node);
    if (lbl == "CmpExpr")    return genBinary(node, true);
    if (lbl == "AddExpr")    return genBinary(node, false);
    if (lbl == "MulExpr")    return genBinary(node, false);

    // Passthrough for single-child nodes
    if (!node->isLeaf && node->children.size() == 1 && !node->children[0]->isLeaf)
        return genExpr(node->children[0].get());

    return "";
}

string IRGenerator::genLiteral(const Node* node) {
    if (node->children.empty()) return "";
    return extractLeafValue(node->children[0].get());
}

string IRGenerator::genAtom(const Node* node) {
    return extractId(node);
}

string IRGenerator::genCallExpr(const Node* node) {
    string funcName = extractId(node);
    auto* argList = findChild(node, "ArgList");

    // Emit PARAM for each argument
    if (argList) {
        for (auto& c : argList->children) {
            if (!c->isLeaf) {
                string argVal = genExpr(c.get());
                emit(IROp::PARAM, argVal);
            }
        }
    }

    string result = newTemp();

    auto it = functions.find(funcName);
    int argCount = 0;
    if (argList) {
        for (auto& c : argList->children)
            if (!c->isLeaf) argCount++;
    }

    if (it != functions.end() && it->second->return_type && !it->second->return_type->isVoid()) {
        emit(IROp::CALL, funcName, to_string(argCount), result);
        return result;
    } else {
        emit(IROp::CALL, funcName, to_string(argCount), "");
        return "";
    }
}

string IRGenerator::genIndexExpr(const Node* node) {
    string arrName = extractId(node);
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "IndexExpr") {
            string idx = genExpr(c.get());
            string result = newTemp();
            emit(IROp::INDEX_LOAD, arrName, idx, result);
            return result;
        }
    }
    return "";
}

string IRGenerator::genArrayLit(const Node* node) {
    string result = newTemp();
    int count = 0;
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            string elem = genExpr(c.get());
            emit(IROp::ARRAY_LIT, elem, to_string(count), result);
            count++;
        }
    }
    return result;
}

string IRGenerator::genRangeExpr(const Node* node) {
    // Just generate both sides; the for loop handles the iteration
    for (auto& c : node->children)
        if (!c->isLeaf) genExpr(c.get());
    return "";
}

string IRGenerator::genRefExpr(const Node* node) {
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            string inner = genExpr(c.get());
            string result = newTemp();
            emit(IROp::REF, inner, "", result);
            return result;
        }
    }
    return "";
}

string IRGenerator::genDerefExpr(const Node* node) {
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            string inner = genExpr(c.get());
            string result = newTemp();
            emit(IROp::DEREF, inner, "", result);
            return result;
        }
    }
    return "";
}

string IRGenerator::genBinary(const Node* node, bool isCmp) {
    vector<const Node*> exprs;
    vector<string> ops;
    for (auto& c : node->children) {
        if (!c->isLeaf) exprs.push_back(c.get());
        else {
            auto val = extractLeafValue(c.get());
            if (!val.empty() && val != "(" && val != ")" && val != "[" && val != "]")
                ops.push_back(val);
        }
    }

    if (exprs.empty()) return "";
    if (exprs.size() == 1) return genExpr(exprs[0]);

    string left = genExpr(exprs[0]);

    for (size_t i = 0; i < ops.size() && i + 1 < exprs.size(); i++) {
        string right = genExpr(exprs[i + 1]);
        string result = newTemp();

        IROp op;
        string& opStr = ops[i];
        if (opStr == "+") op = IROp::ADD;
        else if (opStr == "-") op = IROp::SUB;
        else if (opStr == "*") op = IROp::MUL;
        else if (opStr == "/") op = IROp::DIV;
        else if (opStr == "==") op = IROp::EQ;
        else if (opStr == "!=") op = IROp::NE;
        else if (opStr == "<") op = IROp::LT;
        else if (opStr == "<=") op = IROp::LE;
        else if (opStr == ">") op = IROp::GT;
        else if (opStr == ">=") op = IROp::GE;
        else op = IROp::NOP;

        emit(op, left, right, result);
        left = result;
    }

    return left;
}

// ============================================================
//  Output
// ============================================================

void IRGenerator::printIR(ostream& os) const {
    int idx = 0;
    for (auto& q : code) {
        os << idx++ << ": " << q.toString() << "\n";
    }
}

void IRGenerator::writeIR(const string& filename) const {
    ofstream ofs(filename);
    if (!ofs.is_open()) {
        cerr << "Error: Cannot open IR output file: " << filename << "\n";
        return;
    }
    printIR(ofs);
    ofs.close();
}
