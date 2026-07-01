#include "../include/SemanticAnalyzer.h"
#include <fstream>

// ============================================================
//  Helpers
// ============================================================

void SemanticAnalyzer::error(const string& msg, int line) {
    errors.emplace_back(msg, line);
}

void SemanticAnalyzer::printErrors(ostream& os) const {
    for (auto& e : errors)
        os << "Error: " << e.message << " at line " << e.line << "\n";
}

// ============================================================
//  IR 输出（作业3 a' 嫁接合并）
// ============================================================

void SemanticAnalyzer::printIR(ostream& os) const {
    int idx = 0;
    for (auto& q : code) {
        os << idx++ << ": " << q.toString() << "\n";
    }
}

void SemanticAnalyzer::writeIR(const string& filename) const {
    ofstream ofs(filename);
    if (!ofs.is_open()) {
        cerr << "Error: Cannot open IR output file: " << filename << "\n";
        return;
    }
    printIR(ofs);
    ofs.close();
}

string SemanticAnalyzer::extractId(const Node* node) const {
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

int SemanticAnalyzer::extractLine(const Node* node) const {
    if (!node) return 0;
    if (node->line > 0) return node->line;
    for (auto& c : node->children) {
        int ln = extractLine(c.get());
        if (ln > 0) return ln;
    }
    return 0;
}

string SemanticAnalyzer::extractLeafValue(const Node* node) const {
    if (!node) return "";
    if (node->isLeaf) {
        auto pos = node->label.find(": ");
        return pos != string::npos ? node->label.substr(pos + 2) : node->label;
    }
    return "";
}

int SemanticAnalyzer::extractInt(const Node* node) const {
    return stoi(extractLeafValue(node));
}

string leafCategory(const Node* node) {
    if (!node || !node->isLeaf) return "";
    auto pos = node->label.find(": ");
    return pos != string::npos ? node->label.substr(0, pos) : node->label;
}

bool isLeafCat(const Node* node, const string& cat) {
    return node && node->isLeaf && leafCategory(node) == cat;
}

// Find first child with given label
const Node* findChild(const Node* node, const string& label) {
    for (auto& c : node->children)
        if (c->label == label) return c.get();
    return nullptr;
}

// Find children with given label
vector<const Node*> findChildren(const Node* node, const string& label) {
    vector<const Node*> result;
    for (auto& c : node->children)
        if (c->label == label) result.push_back(c.get());
    return result;
}

// Check if a leaf child has a specific category prefix
bool hasLeafChild(const Node* node, const string& cat) {
    for (auto& c : node->children)
        if (c->isLeaf && leafCategory(c.get()) == cat) return true;
    return false;
}

shared_ptr<Symbol> SemanticAnalyzer::lookupVar(const string& name, int line) {
    auto sym = symtab.lookup(name);
    if (!sym) {
        error("variable '" + name + "' not declared", line);
    }
    return sym;
}

STypePtr SemanticAnalyzer::parseTypeFromNode(const Node* node) {
    if (!node || node->label != "Type") return SType::makeUnknown();

    auto& children = node->children;
    if (children.empty()) return SType::makeUnknown();

    // First child determines the type
    auto& first = children[0];

    if (isLeafCat(first.get(), "I32")) {
        return SType::makeI32();
    }

    if (isLeafCat(first.get(), "Identifier")) {
        string name = extractLeafValue(first.get());
        if (name == "i32") return SType::makeI32();
        return SType::makeUnknown();
    }

    if (isLeafCat(first.get(), "LBracket")) {
        // [Type; NUM]
        auto elemType = parseTypeFromNode(children[1].get());
        int size = extractInt(children[3].get());
        return SType::makeArray(elemType, size);
    }

    if (isLeafCat(first.get(), "Ampersand")) {
        // &T or &mut T
        int typeIdx = 1;
        bool is_mut = (children.size() > 2 && isLeafCat(children[1].get(), "Mut"));
        if (is_mut) typeIdx = 2;
        auto innerType = parseTypeFromNode(children[typeIdx].get());
        return is_mut ? SType::makeMutRef(innerType) : SType::makeRef(innerType);
    }

    if (isLeafCat(first.get(), "LParen")) {
        // () or (T, T, ...)
        if (children.size() <= 2 || isLeafCat(children[1].get(), "RParen"))
            return SType::makeUnit();

        // Check if it's (T,) or (T, T, ...) — has commas
        vector<STypePtr> types;
        for (size_t i = 1; i < children.size(); i++) {
            if (children[i]->label == "Type")
                types.push_back(parseTypeFromNode(children[i].get()));
        }
        if (types.empty()) return SType::makeUnit();
        if (types.size() == 1 && !hasLeafChild(node, "Comma"))
            return types[0]; // just (T) — parenthesized type
        return SType::makeTuple(move(types));
    }

    return SType::makeUnknown();
}

// ============================================================
//  Top-level visitors
// ============================================================

void SemanticAnalyzer::analyze(const Node* root) {
    if (root && root->label == "Program") visitProgram(root);
}

void SemanticAnalyzer::visitProgram(const Node* node) {
    auto funcNodes = findChildren(node, "Function");
    // Pass 1: register all function signatures
    for (auto* fn : funcNodes)
        registerFunction(fn);
    // Pass 2: analyze function bodies
    for (auto* fn : funcNodes)
        visitFunction(fn);
}

void SemanticAnalyzer::registerFunction(const Node* node) {
    auto* header = findChild(node, "FuncHeader");
    if (!header) return;

    string funcName;
    vector<pair<string, STypePtr>> params;
    STypePtr retType = SType::makeVoid();
    auto& hc = header->children;
    funcName = extractLeafValue(hc[1].get());

    auto* paramList = findChild(header, "ParamList");
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
        if (isLeafCat(hc[i].get(), "Arrow") && i + 1 < hc.size()) {
            retType = parseTypeFromNode(hc[i + 1].get());
            break;
        }
    }

    functions[funcName] = make_shared<FunctionInfo>(funcName, params, retType, extractLine(header));
}

void SemanticAnalyzer::visitFunction(const Node* node) {
    auto* header = findChild(node, "FuncHeader");
    auto* block = findChild(node, "Block");
    if (!header) return;

    string funcName = extractLeafValue(header->children[1].get());
    auto it = functions.find(funcName);
    if (it == functions.end()) return;
    current_function = it->second;

    // IR: 函数开始
    emit(IROp::FUNC_BEGIN, funcName);

    symtab.enterScope();
    for (auto& [pname, ptype] : current_function->params) {
        auto sym = make_shared<Symbol>(pname, ptype, true, symtab.scopeLevel(), extractLine(header));
        sym->is_assigned = true;
        symtab.insert(sym);
        // IR: 形参赋值
        emit(IROp::ASSIGN, "param_" + pname, "", pname);
    }

    if (block) {
        block_tail_as_return = true;     // 7.2：函数体末尾表达式作隐式 return
        visitBlock(block);
        block_tail_as_return = false;
    }

    symtab.exitScope();

    // IR: 函数结束标签 + 结束标记
    emit(IROp::LABEL, "func_" + funcName + "_end");
    emit(IROp::FUNC_END, funcName);

    current_function = nullptr;
}

void SemanticAnalyzer::visitBlock(const Node* node) {
    symtab.enterScope();
    bool amFuncBody = block_tail_as_return;
    block_tail_as_return = false;                      // 嵌套块（then/else/loop body）不是函数体
    node_block_type[node] = SType::makeUnit();         // 默认块值为 unit
    for (auto& c : node->children) {
        if (c->isLeaf) continue; // skip { } tokens
        if (c->label == "TailExpr") {
            // 7.0/7.1 块末尾表达式（块值）
            const Node* tailExpr = nullptr;
            for (auto& tc : c->children)
                if (!tc->isLeaf) { tailExpr = tc.get(); break; }
            if (tailExpr) {
                auto t = visitExpr(tailExpr);
                node_ir_temp[node] = node_ir_temp[tailExpr];
                node_block_type[node] = t;            // 块值类型
                // 7.2 仅函数体直接块的末尾表达式作为隐式 return（有值才 return）
                if (amFuncBody && !node_ir_temp[tailExpr].empty()) {
                    emit(IROp::RETURN, node_ir_temp[tailExpr]);
                    if (current_function)
                        emit(IROp::JUMP, "func_" + current_function->name + "_end");
                }
            }
        } else {
            visitStmt(c.get());
        }
    }
    block_tail_as_return = amFuncBody;
    symtab.exitScope();
}

// ============================================================
//  Statement visitors
// ============================================================

void SemanticAnalyzer::visitStmt(const Node* node) {
    const string& lbl = node->label;
    if (lbl == "EmptyStmt") { /* nothing */ }
    else if (lbl == "LetStmt")    visitLetStmt(node);
    else if (lbl == "AssignStmt") visitAssignStmt(node);
    else if (lbl == "ReturnStmt") visitReturnStmt(node);
    else if (lbl == "IfStmt")     visitIfStmt(node);
    else if (lbl == "WhileStmt")  visitWhileStmt(node);
    else if (lbl == "ForStmt")    visitForStmt(node);
    else if (lbl == "LoopStmt")   visitLoopStmt(node);
    else if (lbl == "BreakStmt")  visitBreakStmt(node);
    else if (lbl == "ContinueStmt") visitContinueStmt(node);
    else if (lbl == "ExprStmt")   visitExprStmt(node);
    else if (lbl == "Block")      visitBlock(node);
}

void SemanticAnalyzer::visitLetStmt(const Node* node) {
    // LetStmt: Let, VarDecl, [=, Expr], Semicolon
    auto* varDecl = findChild(node, "VarDecl");
    if (!varDecl) return;

    int line = extractLine(node);
    auto& vc = varDecl->children;

    bool is_mut = hasLeafChild(varDecl, "Mut");
    int idIdx = is_mut ? 1 : 0;
    string varName = extractLeafValue(vc[idIdx].get());

    // Type annotation
    STypePtr varType = SType::makeUnknown();
    auto* typeNode = findChild(varDecl, "Type");
    if (typeNode) varType = parseTypeFromNode(typeNode);

    // Check for initializer
    bool hasInit = hasLeafChild(node, "Assign");
    STypePtr initType = SType::makeUnknown();

    if (hasInit) {
        // Find the Expr child (skip Let/VarDecl/leaf tokens)
        for (auto& c : node->children) {
            if (!c->isLeaf && c->label != "VarDecl") {
                initType = visitExpr(c.get());
                // IR: 声明时赋值（对齐 genLetStmt，结果与变量名不同才 emit）
                string r = node_ir_temp[c.get()];
                if (!r.empty() && r != varName) emit(IROp::ASSIGN, r, "", varName);
                break;
            }
        }

        // Check: void function cannot be used as rvalue
        if (initType->isVoid()) {
            error("cannot use void expression as initializer", line);
        }

        if (varType->isUnknown()) {
            // Infer type from initializer
            varType = initType;
        } else {
            // Check type consistency
            if (!initType->isUnknown() && !varType->equals(initType)) {
                error("type mismatch in variable declaration: expected " +
                      varType->toString() + " but got " + initType->toString(), line);
            }
        }
    }

    // Shadowing: just insert (SymbolTable allows overwriting in current scope)
    auto sym = make_shared<Symbol>(varName, varType, is_mut, symtab.scopeLevel(), line);
    sym->is_assigned = hasInit;
    symtab.insert(sym);
}

void SemanticAnalyzer::visitAssignStmt(const Node* node) {
    // AssignStmt: LHSExpr, Assign, RHSExpr, Semicolon
    int line = extractLine(node);

    // Find LHS and RHS expressions
    int assignIdx = -1;
    for (size_t i = 0; i < node->children.size(); i++) {
        if (isLeafCat(node->children[i].get(), "Assign")) {
            assignIdx = (int)i;
            break;
        }
    }

    if (assignIdx < 0) return;

    auto* lhsNode = node->children[0].get();

    // LHS 检查（抑制 IR emit：对齐 genAssignStmt 不对 lhs 整体求值，避免多余的 INDEX_LOAD/DEREF）
    emit_enabled = false;
    checkLvalue(lhsNode, line);

    STypePtr lhsType;
    if (lhsNode->label == "Identifier") {
        string name = extractId(lhsNode);
        auto sym = symtab.lookup(name);
        if (sym) {
            lhsType = sym->type;
            if (!sym->is_mutable) {
                error("cannot assign to immutable variable '" + name + "'", line);
            }
            if (sym->type->isRef() && !sym->type->isMutRef()) {
                error("cannot assign through immutable reference", line);
            }
        }
    } else {
        lhsType = visitExpr(lhsNode);   // 仅取类型，emit 已抑制
    }
    emit_enabled = true;

    // RHS（恢复 emit，求值右值）
    auto* rhsNode = node->children[assignIdx + 1].get();
    auto rhsType = visitExpr(rhsNode);
    string rhs = node_ir_temp[rhsNode];

    // Type check
    if (lhsType && !lhsType->isUnknown() && !rhsType->isUnknown() && !lhsType->equals(rhsType)) {
        error("type mismatch in assignment: " + lhsType->toString() +
              " vs " + rhsType->toString(), line);
    }

    // IR: 赋值存储（对齐 genAssignStmt，按 lhs 形态 emit）
    if (lhsNode->label == "Identifier") {
        string name = extractId(lhsNode);
        if (rhs != name) emit(IROp::ASSIGN, rhs, "", name);
    } else if (lhsNode->label == "IndexExpr") {
        string arrName = extractId(lhsNode);
        for (auto& c : lhsNode->children) {
            if (!c->isLeaf && c->label != "IndexExpr") {
                visitExpr(c.get());  // 求值索引（emit）
                emit(IROp::INDEX_STORE, rhs, node_ir_temp[c.get()], arrName);
                break;
            }
        }
    } else if (lhsNode->label == "DerefExpr") {
        const Node* ptrNode = lhsNode->children.size() > 1 ? lhsNode->children[1].get() : nullptr;
        if (ptrNode) {
            visitExpr(ptrNode);  // 求值指针（emit）
            emit(IROp::DEREF, rhs, "", node_ir_temp[ptrNode]);
        }
    }

    // Mark LHS as assigned
    if (lhsNode->label == "Identifier") {
        string name = extractId(lhsNode);
        auto sym = symtab.lookup(name);
        if (sym) sym->is_assigned = true;
    }
}

void SemanticAnalyzer::visitReturnStmt(const Node* node) {
    // ReturnStmt: Return, [Expr], Semicolon
    int line = extractLine(node);
    STypePtr retType = SType::makeVoid();
    const Node* retNode = nullptr;

    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "ReturnStmt") {
            retType = visitExpr(c.get());
            retNode = c.get();
            break;
        }
    }

    // IR: 返回（对齐 genReturnStmt：有值 emit RETURN val；无值 emit RETURN + 跳函数结束）
    if (retNode) {
        emit(IROp::RETURN, node_ir_temp[retNode]);
    } else {
        emit(IROp::RETURN);
        if (current_function) emit(IROp::JUMP, "func_" + current_function->name + "_end");
    }

    if (current_function) {
        auto funcRet = current_function->return_type;
        if (!funcRet) funcRet = SType::makeVoid();

        if (funcRet->isVoid() && !retType->isVoid()) {
            error("return statement type (" + retType->toString() +
                  ") does not match function return type (void)", line);
        } else if (!funcRet->isVoid() && retType->isVoid()) {
            error("return statement type (void) does not match function return type (" +
                  funcRet->toString() + ")", line);
        } else if (!funcRet->isVoid() && !retType->isVoid() && !funcRet->equals(retType)) {
            error("return type mismatch: expected " + funcRet->toString() +
                  " but got " + retType->toString(), line);
        }
    }
}

void SemanticAnalyzer::visitIfStmt(const Node* node) {
    // IfStmt: If, Expr, Block, [ElseClause]
    int line = extractLine(node);
    (void)line;

    // IR: 分配 else/end 标签（对齐 genIfStmt）
    string elseLabel = newLabel();
    string endLabel = newLabel();

    // Condition expression
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "IfStmt" && c->label != "Block" &&
            c->label != "ElseClause") {
            visitExpr(c.get());
            emit(IROp::JZ, node_ir_temp[c.get()], elseLabel);  // IR
            break;
        }
    }

    // Then block
    auto blocks = findChildren(node, "Block");
    if (!blocks.empty()) visitBlock(blocks[0]);
    emit(IROp::JUMP, endLabel);  // IR

    // Else label + else clause
    emit(IROp::LABEL, elseLabel);  // IR
    auto* elseClause = findChild(node, "ElseClause");
    if (elseClause) {
        for (auto& c : elseClause->children) {
            if (!c->isLeaf) {
                if (c->label == "Block") visitBlock(c.get());
                else if (c->label == "IfStmt") visitIfStmt(c.get());
            }
        }
    }

    emit(IROp::LABEL, endLabel);  // IR
}

void SemanticAnalyzer::visitWhileStmt(const Node* node) {
    // WhileStmt: While, Expr, Block
    string startLabel = newLabel();
    string endLabel = newLabel();

    emit(IROp::LABEL, startLabel);  // IR

    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "WhileStmt" && c->label != "Block") {
            visitExpr(c.get());
            emit(IROp::JZ, node_ir_temp[c.get()], endLabel);  // IR
            break;
        }
    }

    string oldBreak = break_label, oldContinue = continue_label;  // IR
    break_label = endLabel;
    continue_label = startLabel;

    auto* block = findChild(node, "Block");
    if (block) {
        in_loop++;
        visitBlock(block);
        in_loop--;
    }

    emit(IROp::JUMP, startLabel);  // IR
    emit(IROp::LABEL, endLabel);   // IR

    break_label = oldBreak;
    continue_label = oldContinue;
}

void SemanticAnalyzer::visitForStmt(const Node* node) {
    // ForStmt: For, [Mut], Identifier, In, Expr, Block
    int line = extractLine(node);

    bool is_mut = false;
    string varName;
    int idIdx = 1;

    for (size_t i = 0; i < node->children.size(); i++) {
        if (isLeafCat(node->children[i].get(), "Mut")) {
            is_mut = true;
            idIdx = i + 1;
        }
    }

    for (auto& c : node->children) {
        if (isLeafCat(c.get(), "Identifier")) {
            varName = extractLeafValue(c.get());
            break;
        }
    }

    // IR: 循环标签（对齐 genForStmt）
    string startLabel = newLabel();
    string endLabel = newLabel();
    string stepLabel = newLabel();
    emit(IROp::LABEL, startLabel);

    // Range expression
    const Node* rangeNode = nullptr;
    for (auto& c : node->children) {
        if (c->label == "RangeExpr") { rangeNode = c.get(); break; }
    }
    if (rangeNode && rangeNode->children.size() >= 3) {
        visitExpr(rangeNode->children[0].get());          // 左端求值（emit + 类型）
        visitExpr(rangeNode->children[2].get());          // 右端求值
        string endTemp = node_ir_temp[rangeNode->children[2].get()];
        if (!endTemp.empty()) {
            string condTemp = newTemp();
            emit(IROp::LT, varName, endTemp, condTemp);
            emit(IROp::JZ, condTemp, endLabel);
        }
    } else {
        // 回退：非 RangeExpr 的范围表达式
        for (auto& c : node->children) {
            if (!c->isLeaf && c->label != "ForStmt" && c->label != "Block" &&
                !isLeafCat(c.get(), "Identifier") && !isLeafCat(c.get(), "For") &&
                !isLeafCat(c.get(), "Mut") && !isLeafCat(c.get(), "In")) {
                visitExpr(c.get());
                break;
            }
        }
    }

    // Insert loop variable
    auto sym = make_shared<Symbol>(varName, SType::makeI32(), is_mut, symtab.scopeLevel() + 1, line);
    sym->is_assigned = true;
    symtab.enterScope();
    symtab.insert(sym);

    // IR: break/continue 指向
    string oldBreak = break_label, oldContinue = continue_label;
    break_label = endLabel;
    continue_label = stepLabel;

    auto* block = findChild(node, "Block");
    if (block) {
        in_loop++;
        visitBlock(block);
        in_loop--;
    }

    symtab.exitScope();

    // IR: 步进 + 自增 + 跳回 + 结束标签
    emit(IROp::LABEL, stepLabel);
    emit(IROp::ADD, varName, "1", varName);
    emit(IROp::JUMP, startLabel);
    emit(IROp::LABEL, endLabel);

    break_label = oldBreak;
    continue_label = oldContinue;
}

void SemanticAnalyzer::visitLoopStmt(const Node* node) {
    string startLabel = newLabel();
    string endLabel = newLabel();

    emit(IROp::LABEL, startLabel);  // IR

    string oldBreak = break_label, oldContinue = continue_label;  // IR
    break_label = endLabel;
    continue_label = startLabel;

    auto* block = findChild(node, "Block");
    if (block) {
        in_loop++;
        visitBlock(block);
        in_loop--;
    }

    emit(IROp::JUMP, startLabel);  // IR
    emit(IROp::LABEL, endLabel);   // IR

    break_label = oldBreak;
    continue_label = oldContinue;
}

void SemanticAnalyzer::visitBreakStmt(const Node* node) {
    int line = extractLine(node);
    if (in_loop <= 0) {
        error("break statement must be inside a loop", line);
    }
    // 7.4 break 可带表达式（循环表达式的返回值）
    const Node* breakExpr = nullptr;
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "BreakStmt") { breakExpr = c.get(); break; }
    }
    if (breakExpr) {
        visitExpr(breakExpr);
        // break 值汇合到 loop_result（循环表达式上下文）
        if (!loop_result.empty() && !node_ir_temp[breakExpr].empty())
            emit(IROp::ASSIGN, node_ir_temp[breakExpr], "", loop_result);
    }
    // IR: 跳到循环结束
    if (!break_label.empty()) emit(IROp::JUMP, break_label);
}

void SemanticAnalyzer::visitContinueStmt(const Node* node) {
    int line = extractLine(node);
    if (in_loop <= 0) {
        error("continue statement must be inside a loop", line);
    }
    // IR: 跳到循环开始
    if (!continue_label.empty()) emit(IROp::JUMP, continue_label);
}

void SemanticAnalyzer::visitExprStmt(const Node* node) {
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            visitExpr(c.get());
            break;
        }
    }
}

// ============================================================
//  7.x 表达式块 / 选择表达式 / 循环表达式
// ============================================================

// 检查 node 内是否有「带表达式的 break」（决定 LoopExpr 是否需要结果汇合 temp）
// 不进入嵌套循环：内层 break 属于内层 loop
static bool hasBreakWithValue(const Node* node) {
    if (!node) return false;
    if (node->label == "BreakStmt") {
        for (auto& c : node->children)
            if (!c->isLeaf && c->label != "BreakStmt") return true;
        return false;
    }
    if (node->label == "LoopExpr" || node->label == "LoopStmt" ||
        node->label == "WhileStmt" || node->label == "ForStmt")
        return false;
    for (auto& c : node->children)
        if (hasBreakWithValue(c.get())) return true;
    return false;
}

STypePtr SemanticAnalyzer::visitIfExpr(const Node* node) {
    string elseLabel = newLabel();
    string endLabel = newLabel();

    // 预检查 then/else 是否含 TailExpr（决定 IfExpr 是否有值）
    auto blocks = findChildren(node, "Block");
    bool hasValue = false;
    for (auto* b : blocks) {
        for (auto& c : b->children)
            if (!c->isLeaf && c->label == "TailExpr") { hasValue = true; break; }
        if (hasValue) break;
    }
    string result = hasValue ? newTemp() : "";

    // 条件
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "IfExpr" && c->label != "Block" &&
            c->label != "ElseClause") {
            visitExpr(c.get());
            emit(IROp::JZ, node_ir_temp[c.get()], elseLabel);
            break;
        }
    }

    // then
    if (!blocks.empty()) {
        visitBlock(blocks[0]);
        if (hasValue && !node_ir_temp[blocks[0]].empty())
            emit(IROp::ASSIGN, node_ir_temp[blocks[0]], "", result);
    }
    emit(IROp::JUMP, endLabel);

    // else
    emit(IROp::LABEL, elseLabel);
    auto* elseClause = findChild(node, "ElseClause");
    if (elseClause) {
        for (auto& c : elseClause->children) {
            if (!c->isLeaf) {
                if (c->label == "Block") {
                    visitBlock(c.get());
                    if (hasValue && !node_ir_temp[c.get()].empty())
                        emit(IROp::ASSIGN, node_ir_temp[c.get()], "", result);
                } else if (c->label == "IfExpr") {
                    visitIfExpr(c.get());
                    if (hasValue && !node_ir_temp[c.get()].empty())
                        emit(IROp::ASSIGN, node_ir_temp[c.get()], "", result);
                }
            }
        }
    }
    emit(IROp::LABEL, endLabel);

    node_ir_temp[node] = result;
    // IfExpr 类型 = then 块的值类型
    return blocks.empty() ? SType::makeUnknown() :
           (node_block_type.count(blocks[0]) ? node_block_type[blocks[0]] : SType::makeUnknown());
}

STypePtr SemanticAnalyzer::visitLoopExpr(const Node* node) {
    string startLabel = newLabel();
    string endLabel = newLabel();

    emit(IROp::LABEL, startLabel);

    auto* block = findChild(node, "Block");
    bool hasBreakValue = block && hasBreakWithValue(block);

    string oldBreak = break_label, oldContinue = continue_label;
    string oldLoopResult = loop_result;
    break_label = endLabel;
    continue_label = startLabel;
    loop_result = hasBreakValue ? newTemp() : "";   // 7.4 break 值汇合点（仅有带值 break 时分配）

    if (block) { in_loop++; visitBlock(block); in_loop--; }

    emit(IROp::JUMP, startLabel);
    emit(IROp::LABEL, endLabel);

    break_label = oldBreak;
    continue_label = oldContinue;
    node_ir_temp[node] = loop_result;
    loop_result = oldLoopResult;
    return hasBreakValue ? SType::makeI32() : SType::makeUnknown();
}

// ============================================================
//  Expression visitors
// ============================================================

STypePtr SemanticAnalyzer::visitExpr(const Node* node) {
    if (!node) return SType::makeUnknown();
    const string& lbl = node->label;

    if (lbl == "CmpExpr")    return visitCmpExpr(node);
    if (lbl == "AddExpr")    return visitAddExpr(node);
    if (lbl == "MulExpr")    return visitMulExpr(node);
    if (lbl == "RefExpr")    return visitRefExpr(node);
    if (lbl == "DerefExpr")  return visitDerefExpr(node);
    if (lbl == "Literal")    return visitLiteral(node);
    if (lbl == "Identifier") return visitAtom(node);
    if (lbl == "CallExpr")   return visitCallExpr(node);
    if (lbl == "IndexExpr")  return visitIndexExpr(node);
    if (lbl == "ArrayLit")   return visitArrayLit(node);
    if (lbl == "TupleLit")   return visitTupleLit(node);
    if (lbl == "ParenExpr")  return visitParenExpr(node);
    if (lbl == "RangeExpr")  return visitRangeExpr(node);
    if (lbl == "IfExpr")     return visitIfExpr(node);
    if (lbl == "LoopExpr")   return visitLoopExpr(node);
    if (lbl == "Block")      { visitBlock(node); return node_block_type.count(node) ? node_block_type[node] : SType::makeUnknown(); }

    // Single child passthrough (e.g. just an AddExpr or Term)
    if (!node->isLeaf && node->children.size() == 1 && !node->children[0]->isLeaf)
        return visitExpr(node->children[0].get());

    return SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitLiteral(const Node* node) {
    // Literal has one child: "IntegerConstant: 42" or similar
    if (node->children.empty()) return SType::makeUnknown();
    auto cat = leafCategory(node->children[0].get());
    if (cat == "IntegerConstant") {
        node_ir_temp[node] = extractLeafValue(node->children[0].get());  // IR
        return SType::makeI32();
    }
    return SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitCmpExpr(const Node* node) {
    // CmpExpr: left, op, right
    int line = extractLine(node);
    auto& ch = node->children;

    vector<const Node*> exprs;
    vector<string> ops;
    for (auto& c : ch) {
        if (!c->isLeaf) exprs.push_back(c.get());
        else {
            auto val = extractLeafValue(c.get());
            if (!val.empty() && val != "(" && val != ")" && val != "[" && val != "]")
                ops.push_back(val);
        }
    }

    if (exprs.size() < 2) {
        if (exprs.size() == 1) {
            auto t = visitExpr(exprs[0]);
            node_ir_temp[node] = node_ir_temp[exprs[0]];
            return t;
        }
        node_ir_temp[node] = "";
        return SType::makeUnknown();
    }

    auto lt = visitExpr(exprs[0]);
    string left = node_ir_temp[exprs[0]];

    for (size_t i = 0; i < ops.size() && i + 1 < exprs.size(); i++) {
        auto rt = visitExpr(exprs[i + 1]);
        string right = node_ir_temp[exprs[i + 1]];

        if (!lt->isUnknown() && !rt->isUnknown() && !lt->equals(rt)) {
            error("comparison operands must have same type: " +
                  lt->toString() + " vs " + rt->toString(), line);
        }

        // IR: 比较运算
        IROp op = IROp::NOP;
        if (ops[i] == "==") op = IROp::EQ;
        else if (ops[i] == "!=") op = IROp::NE;
        else if (ops[i] == "<") op = IROp::LT;
        else if (ops[i] == "<=") op = IROp::LE;
        else if (ops[i] == ">") op = IROp::GT;
        else if (ops[i] == ">=") op = IROp::GE;
        string result = newTemp();
        emit(op, left, right, result);
        left = result;
    }

    node_ir_temp[node] = left;
    return SType::makeBool();
}

STypePtr SemanticAnalyzer::visitAddExpr(const Node* node) {
    int line = extractLine(node);
    auto& ch = node->children;
    vector<const Node*> exprs;
    vector<string> ops;
    for (auto& c : ch) {
        if (!c->isLeaf) exprs.push_back(c.get());
        else {
            auto val = extractLeafValue(c.get());
            if (!val.empty() && val != "(" && val != ")" && val != "[" && val != "]")
                ops.push_back(val);
        }
    }

    if (exprs.size() < 2) {
        if (exprs.size() == 1) {
            auto t = visitExpr(exprs[0]);
            node_ir_temp[node] = node_ir_temp[exprs[0]];
            return t;
        }
        node_ir_temp[node] = "";
        return SType::makeUnknown();
    }

    auto lt = visitExpr(exprs[0]);
    string left = node_ir_temp[exprs[0]];
    STypePtr rt;

    for (size_t i = 0; i < ops.size() && i + 1 < exprs.size(); i++) {
        rt = visitExpr(exprs[i + 1]);
        string right = node_ir_temp[exprs[i + 1]];

        if (!lt->isUnknown() && !rt->isUnknown() && !lt->equals(rt)) {
            error("arithmetic operands must have same type: " +
                  lt->toString() + " vs " + rt->toString(), line);
        }

        // IR: 加减运算
        IROp op = (ops[i] == "+") ? IROp::ADD : IROp::SUB;
        string result = newTemp();
        emit(op, left, right, result);
        left = result;
    }

    node_ir_temp[node] = left;
    return lt->isUnknown() ? (rt ? rt : lt) : lt;
}

STypePtr SemanticAnalyzer::visitMulExpr(const Node* node) {
    int line = extractLine(node);
    auto& ch = node->children;
    vector<const Node*> exprs;
    vector<string> ops;
    for (auto& c : ch) {
        if (!c->isLeaf) exprs.push_back(c.get());
        else {
            auto val = extractLeafValue(c.get());
            if (!val.empty() && val != "(" && val != ")" && val != "[" && val != "]")
                ops.push_back(val);
        }
    }

    if (exprs.size() < 2) {
        if (exprs.size() == 1) {
            auto t = visitExpr(exprs[0]);
            node_ir_temp[node] = node_ir_temp[exprs[0]];
            return t;
        }
        node_ir_temp[node] = "";
        return SType::makeUnknown();
    }

    auto lt = visitExpr(exprs[0]);
    string left = node_ir_temp[exprs[0]];
    STypePtr rt;

    for (size_t i = 0; i < ops.size() && i + 1 < exprs.size(); i++) {
        rt = visitExpr(exprs[i + 1]);
        string right = node_ir_temp[exprs[i + 1]];

        if (!lt->isUnknown() && !rt->isUnknown() && !lt->equals(rt)) {
            error("arithmetic operands must have same type: " +
                  lt->toString() + " vs " + rt->toString(), line);
        }

        // IR: 乘除运算
        IROp op = (ops[i] == "*") ? IROp::MUL : IROp::DIV;
        string result = newTemp();
        emit(op, left, right, result);
        left = result;
    }

    node_ir_temp[node] = left;
    return lt->isUnknown() ? (rt ? rt : lt) : lt;
}

STypePtr SemanticAnalyzer::visitRefExpr(const Node* node) {
    // RefExpr: &, [mut], inner
    int line = extractLine(node);
    bool is_mut = hasLeafChild(node, "Mut");

    const Node* innerExpr = nullptr;
    for (auto& c : node->children) {
        if (!c->isLeaf) { innerExpr = c.get(); break; }
    }

    if (!innerExpr) return SType::makeUnknown();

    auto innerType = visitExpr(innerExpr);

    // IR: 创建引用 REF inner, _, result
    {
        string result = newTemp();
        emit(IROp::REF, node_ir_temp[innerExpr], "", result);
        node_ir_temp[node] = result;
    }

    // Check that the referenced variable is mutable if creating mutable ref
    if (innerExpr->label == "Identifier") {
        string name = extractId(innerExpr);
        auto sym = symtab.lookup(name);
        if (is_mut && sym && !sym->is_mutable) {
            error("cannot create mutable reference to immutable variable '" + name + "'", line);
        }
        // Borrow tracking: mutable ref cannot coexist with other refs
        if (sym) {
            if (is_mut) {
                if (sym->has_immutable_ref || sym->has_mutable_ref) {
                    error("cannot create mutable reference to '" + name +
                          "': other references already exist", line);
                }
                sym->has_mutable_ref = true;
            } else {
                if (sym->has_mutable_ref) {
                    error("cannot create immutable reference to '" + name +
                          "': mutable reference already exists", line);
                }
                sym->has_immutable_ref = true;
            }
        }
    }

    return is_mut ? SType::makeMutRef(innerType) : SType::makeRef(innerType);
}

STypePtr SemanticAnalyzer::visitDerefExpr(const Node* node) {
    // DerefExpr: *, inner
    int line = extractLine(node);
    const Node* innerExpr = nullptr;
    for (auto& c : node->children) {
        if (!c->isLeaf) { innerExpr = c.get(); break; }
    }

    if (!innerExpr) return SType::makeUnknown();
    auto innerType = visitExpr(innerExpr);

    // IR: 解引用 DEREF inner, _, result
    {
        string result = newTemp();
        emit(IROp::DEREF, node_ir_temp[innerExpr], "", result);
        node_ir_temp[node] = result;
    }

    if (!innerType->isRef() && !innerType->isMutRef()) {
        error("cannot dereference non-reference type: " + innerType->toString(), line);
        return SType::makeUnknown();
    }

    return innerType->inner ? innerType->inner : SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitCallExpr(const Node* node) {
    // CallExpr: Identifier, LParen, ArgList, RParen
    int line = extractLine(node);
    string funcName = extractId(node);

    // Process arguments
    auto* argList = findChild(node, "ArgList");
    vector<STypePtr> argTypes;
    int argCount = 0;
    if (argList) {
        for (auto& c : argList->children) {
            if (!c->isLeaf) {
                argTypes.push_back(visitExpr(c.get()));
                emit(IROp::PARAM, node_ir_temp[c.get()]);  // IR: 参数
                argCount++;
            }
        }
    }

    // IR: 调用结果临时变量（对齐 genCallExpr，在 PARAM 之后、CALL 之前分配）
    string irResult = newTemp();

    auto it = functions.find(funcName);
    if (it == functions.end()) {
        error("function '" + funcName + "' not declared", line);
        emit(IROp::CALL, funcName, to_string(argCount), "");  // IR
        node_ir_temp[node] = "";
        return SType::makeUnknown();
    }

    auto& func = it->second;

    // Check argument count
    if (argTypes.size() != func->params.size()) {
        error("function '" + funcName + "' expects " +
              to_string(func->params.size()) + " argument(s) but got " +
              to_string(argTypes.size()), line);
    } else {
        // Check argument types
        for (size_t i = 0; i < argTypes.size() && i < func->params.size(); i++) {
            auto& paramType = func->params[i].second;
            if (!argTypes[i]->isUnknown() && !paramType->isUnknown() &&
                !argTypes[i]->equals(paramType)) {
                error("argument " + to_string(i + 1) + " type mismatch in call to '" +
                      funcName + "': expected " + paramType->toString() +
                      " but got " + argTypes[i]->toString(), line);
            }
        }
    }

    // IR: CALL（有返回值则写入结果临时变量，否则空）
    if (func->return_type && !func->return_type->isVoid()) {
        emit(IROp::CALL, funcName, to_string(argCount), irResult);
        node_ir_temp[node] = irResult;
    } else {
        emit(IROp::CALL, funcName, to_string(argCount), "");
        node_ir_temp[node] = "";
    }

    return func->return_type ? func->return_type : SType::makeVoid();
}

STypePtr SemanticAnalyzer::visitIndexExpr(const Node* node) {
    // IndexExpr: Identifier, LBracket, Expr, RBracket
    int line = extractLine(node);
    string varName = extractId(node);

    auto sym = lookupVar(varName, line);
    if (!sym) { node_ir_temp[node] = ""; return SType::makeUnknown(); }

    if (!sym->is_assigned) {
        error("variable '" + varName + "' used before assignment", line);
    }

    // Index expression
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "IndexExpr") {
            auto idxType = visitExpr(c.get());
            if (!idxType->isUnknown() && !idxType->isI32()) {
                error("array index must be integer type", line);
            }
            // IR: 数组加载 INDEX_LOAD arrName, idx, result
            {
                string result = newTemp();
                emit(IROp::INDEX_LOAD, varName, node_ir_temp[c.get()], result);
                node_ir_temp[node] = result;
            }
            break;
        }
    }

    auto& arrType = sym->type;
    if (arrType->isArray()) {
        return arrType->inner ? arrType->inner : SType::makeUnknown();
    }
    if (arrType->isTuple()) {
        return SType::makeUnknown(); // tuple index uses . not []
    }

    error("cannot index non-array type: " + arrType->toString(), line);
    return SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitArrayLit(const Node* node) {
    // ArrayLit: LBracket, [exprs...], RBracket
    int line = extractLine(node);
    vector<STypePtr> elemTypes;

    // IR: 数组字面量 ARRAY_LIT elem, count, result
    string irResult = newTemp();
    int count = 0;
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            elemTypes.push_back(visitExpr(c.get()));
            emit(IROp::ARRAY_LIT, node_ir_temp[c.get()], to_string(count), irResult);
            count++;
        }
    }
    node_ir_temp[node] = irResult;

    if (elemTypes.empty()) return SType::makeArray(SType::makeUnknown(), 0);

    // All elements should have the same type
    auto& first = elemTypes[0];
    for (size_t i = 1; i < elemTypes.size(); i++) {
        if (!first->isUnknown() && !elemTypes[i]->isUnknown() && !first->equals(elemTypes[i])) {
            error("array literal elements must have same type: " +
                  first->toString() + " vs " + elemTypes[i]->toString(), line);
        }
    }

    return SType::makeArray(first->isUnknown() ? SType::makeUnknown() : first, (int)elemTypes.size());
}

STypePtr SemanticAnalyzer::visitTupleLit(const Node* node) {
    // TupleLit: LParen, [exprs with commas...], RParen
    int line = extractLine(node);

    // Check if it's empty tuple ()
    bool hasComma = hasLeafChild(node, "Comma");
    if (!hasComma) {
        // Check children: only LParen and RParen => unit
        int nonLeafCount = 0;
        for (auto& c : node->children) {
            if (!c->isLeaf) nonLeafCount++;
        }
        if (nonLeafCount == 0) { node_ir_temp[node] = ""; return SType::makeUnit(); }
        // Single element without comma => not a tuple (shouldn't reach here, parser handles it)
    }

    vector<STypePtr> elemTypes;

    // IR: 元组字面量（复用 ARRAY_LIT emit，对齐 IRGenerator genArrayLit）
    string irResult = newTemp();
    int count = 0;
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            elemTypes.push_back(visitExpr(c.get()));
            emit(IROp::ARRAY_LIT, node_ir_temp[c.get()], to_string(count), irResult);
            count++;
        }
    }
    node_ir_temp[node] = irResult;

    return SType::makeTuple(move(elemTypes));
}

STypePtr SemanticAnalyzer::visitRangeExpr(const Node* node) {
    // RangeExpr: left, .., right
    for (auto& c : node->children) {
        if (!c->isLeaf) visitExpr(c.get());
    }
    node_ir_temp[node] = "";  // IR: range 本身不求值（for 循环处理两端）
    return SType::makeI32(); // ranges are integer
}

STypePtr SemanticAnalyzer::visitParenExpr(const Node* node) {
    // ParenExpr: (, Expr, )
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            auto t = visitExpr(c.get());
            node_ir_temp[node] = node_ir_temp[c.get()];  // IR 透传
            return t;
        }
    }
    node_ir_temp[node] = "";
    return SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitAtom(const Node* node) {
    int line = extractLine(node);

    // Identifier node: single leaf child "Identifier: name"
    if (node->label == "Identifier") {
        string name = extractId(node);
        node_ir_temp[node] = name;  // IR
        auto sym = lookupVar(name, line);
        if (!sym) return SType::makeUnknown();

        if (!sym->is_assigned) {
            error("variable '" + name + "' used before assignment", line);
        }
        return sym->type;
    }

    node_ir_temp[node] = "";
    return SType::makeUnknown();
}

// ============================================================
//  Lvalue checking
// ============================================================

void SemanticAnalyzer::checkLvalue(const Node* node, int line) {
    if (!node) return;

    if (node->label == "Identifier") {
        string name = extractId(node);
        auto sym = symtab.lookup(name);
        if (!sym) {
            error("variable '" + name + "' not declared", line);
        }
        return;
    }

    if (node->label == "IndexExpr") {
        // Array element is lvalue if array is mutable
        string name = extractId(node);
        auto sym = symtab.lookup(name);
        if (sym && !sym->is_mutable) {
            error("cannot assign to element of immutable array '" + name + "'", line);
        }
        return;
    }

    if (node->label == "DerefExpr") {
        // *ptr is lvalue if ptr is mutable reference
        const Node* inner = nullptr;
        for (auto& c : node->children)
            if (!c->isLeaf) { inner = c.get(); break; }

        if (inner) {
            auto innerType = visitExpr(inner);
            if (innerType->isRef() && !innerType->isMutRef()) {
                error("cannot assign through immutable reference", line);
            }
        }
        return;
    }
}
