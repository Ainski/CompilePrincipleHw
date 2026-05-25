#include "../include/SemanticAnalyzer.h"

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
    for (auto& c : node->children)
        if (c->label == "Function") visitFunction(c.get());
}

void SemanticAnalyzer::visitFunction(const Node* node) {
    auto* header = findChild(node, "FuncHeader");
    auto* block = findChild(node, "Block");
    if (!header) return;

    // Parse function header
    string funcName;
    vector<pair<string, STypePtr>> params;
    STypePtr retType = SType::makeVoid();

    auto& hc = header->children;

    // hc: Fn, Identifier, LParen, ParamList, RParen, [Arrow, Type]
    funcName = extractLeafValue(hc[1].get());

    // Process ParamList
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

    // Check for return type
    for (size_t i = 0; i < hc.size(); i++) {
        if (isLeafCat(hc[i].get(), "Arrow") && i + 1 < hc.size()) {
            retType = parseTypeFromNode(hc[i + 1].get());
            break;
        }
    }

    auto funcInfo = make_shared<FunctionInfo>(funcName, params, retType, extractLine(header));
    functions[funcName] = funcInfo;
    current_function = funcInfo;

    // Enter function scope, insert params
    symtab.enterScope();
    for (auto& [pname, ptype] : params) {
        auto sym = make_shared<Symbol>(pname, ptype, true, symtab.scopeLevel(), extractLine(header));
        sym->is_assigned = true; // params are initialized
        symtab.insert(sym);
    }

    if (block) visitBlock(block);

    symtab.exitScope();
    current_function = nullptr;
}

void SemanticAnalyzer::visitBlock(const Node* node) {
    symtab.enterScope();
    for (auto& c : node->children) {
        if (c->isLeaf) continue; // skip { } tokens
        visitStmt(c.get());
    }
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
        // Find the Expr child
        for (auto& c : node->children) {
            if (c->label != "LetStmt" && c->label != "VarDecl" && !c->isLeaf &&
                c->label != "EmptyStmt") {
                // This should be the expression
                if (c->label.find("Expr") != string::npos || c->label == "Literal" ||
                    c->label == "Identifier" || c->label == "CallExpr" ||
                    c->label == "ArrayLit" || c->label == "ParenExpr" ||
                    c->label == "RefExpr" || c->label == "DerefExpr" ||
                    c->label == "IndexExpr" || c->label == "RangeExpr") {
                    initType = visitExpr(c.get());
                    break;
                }
            }
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

    // Check LHS is declared and valid lvalue (but NOT "used before assignment")
    checkLvalue(lhsNode, line);

    // Check mutability before evaluating LHS type
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
        lhsType = visitExpr(lhsNode);
    }

    // RHS
    auto rhsType = visitExpr(node->children[assignIdx + 1].get());

    // Type check
    if (lhsType && !lhsType->isUnknown() && !rhsType->isUnknown() && !lhsType->equals(rhsType)) {
        error("type mismatch in assignment: " + lhsType->toString() +
              " vs " + rhsType->toString(), line);
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

    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "ReturnStmt") {
            retType = visitExpr(c.get());
            break;
        }
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

    // Condition expression
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "IfStmt" && c->label != "Block" &&
            c->label != "ElseClause") {
            visitExpr(c.get());
            break;
        }
    }

    // Then block
    auto blocks = findChildren(node, "Block");
    for (auto* b : blocks) visitBlock(b);

    // Else clause
    auto* elseClause = findChild(node, "ElseClause");
    if (elseClause) {
        for (auto& c : elseClause->children) {
            if (!c->isLeaf) {
                if (c->label == "Block") visitBlock(c.get());
                else if (c->label == "IfStmt") visitIfStmt(c.get());
            }
        }
    }
}

void SemanticAnalyzer::visitWhileStmt(const Node* node) {
    // WhileStmt: While, Expr, Block
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "WhileStmt" && c->label != "Block") {
            visitExpr(c.get());
            break;
        }
    }
    auto* block = findChild(node, "Block");
    if (block) {
        in_loop++;
        visitBlock(block);
        in_loop--;
    }
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

    // Range expression
    for (auto& c : node->children) {
        if (c->label == "RangeExpr" || (!c->isLeaf &&
            c->label != "ForStmt" && c->label != "Block" &&
            !isLeafCat(c.get(), "Identifier") && !isLeafCat(c.get(), "For") &&
            !isLeafCat(c.get(), "Mut") && !isLeafCat(c.get(), "In"))) {
            auto rangeType = visitExpr(c.get());
            // Range bounds should be integer type
            break;
        }
    }

    // Insert loop variable
    auto sym = make_shared<Symbol>(varName, SType::makeI32(), is_mut, symtab.scopeLevel() + 1, line);
    sym->is_assigned = true;
    symtab.enterScope();
    symtab.insert(sym);

    auto* block = findChild(node, "Block");
    if (block) {
        in_loop++;
        visitBlock(block);
        in_loop--;
    }

    symtab.exitScope();
}

void SemanticAnalyzer::visitLoopStmt(const Node* node) {
    auto* block = findChild(node, "Block");
    if (block) {
        in_loop++;
        visitBlock(block);
        in_loop--;
    }
}

void SemanticAnalyzer::visitBreakStmt(const Node* node) {
    int line = extractLine(node);
    if (in_loop <= 0) {
        error("break statement must be inside a loop", line);
    }
    // Check if break has an expression (for loop expressions 7.4)
    for (auto& c : node->children) {
        if (!c->isLeaf && c->label != "BreakStmt") {
            visitExpr(c.get());
            break;
        }
    }
}

void SemanticAnalyzer::visitContinueStmt(const Node* node) {
    int line = extractLine(node);
    if (in_loop <= 0) {
        error("continue statement must be inside a loop", line);
    }
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

    // Single child passthrough (e.g. just an AddExpr or Term)
    if (!node->isLeaf && node->children.size() == 1 && !node->children[0]->isLeaf)
        return visitExpr(node->children[0].get());

    return SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitLiteral(const Node* node) {
    // Literal has one child: "IntegerConstant: 42" or similar
    if (node->children.empty()) return SType::makeUnknown();
    auto cat = leafCategory(node->children[0].get());
    if (cat == "IntegerConstant") return SType::makeI32();
    return SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitCmpExpr(const Node* node) {
    // CmpExpr: left, op, right
    int line = extractLine(node);
    auto& ch = node->children;

    // Find the two expression children and the operator
    vector<const Node*> exprs;
    for (auto& c : ch) {
        if (!c->isLeaf) exprs.push_back(c.get());
    }

    if (exprs.size() < 2) {
        if (exprs.size() == 1) return visitExpr(exprs[0]);
        return SType::makeUnknown();
    }

    auto lt = visitExpr(exprs[0]);
    auto rt = visitExpr(exprs[1]);

    if (!lt->isUnknown() && !rt->isUnknown() && !lt->equals(rt)) {
        error("comparison operands must have same type: " +
              lt->toString() + " vs " + rt->toString(), line);
    }

    return SType::makeBool();
}

STypePtr SemanticAnalyzer::visitAddExpr(const Node* node) {
    auto& ch = node->children;
    vector<const Node*> exprs;
    for (auto& c : ch)
        if (!c->isLeaf) exprs.push_back(c.get());

    if (exprs.size() < 2) {
        return exprs.size() == 1 ? visitExpr(exprs[0]) : SType::makeUnknown();
    }

    auto lt = visitExpr(exprs[0]);
    auto rt = visitExpr(exprs[1]);
    int line = extractLine(node);

    if (!lt->isUnknown() && !rt->isUnknown() && !lt->equals(rt)) {
        error("arithmetic operands must have same type: " +
              lt->toString() + " vs " + rt->toString(), line);
    }

    return lt->isUnknown() ? rt : lt;
}

STypePtr SemanticAnalyzer::visitMulExpr(const Node* node) {
    auto& ch = node->children;
    vector<const Node*> exprs;
    for (auto& c : ch)
        if (!c->isLeaf) exprs.push_back(c.get());

    if (exprs.size() < 2) {
        return exprs.size() == 1 ? visitExpr(exprs[0]) : SType::makeUnknown();
    }

    auto lt = visitExpr(exprs[0]);
    auto rt = visitExpr(exprs[1]);
    int line = extractLine(node);

    if (!lt->isUnknown() && !rt->isUnknown() && !lt->equals(rt)) {
        error("arithmetic operands must have same type: " +
              lt->toString() + " vs " + rt->toString(), line);
    }

    return lt->isUnknown() ? rt : lt;
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

    // Check that the referenced variable is mutable if creating mutable ref
    if (is_mut && innerExpr->label == "Identifier") {
        string name = extractId(innerExpr);
        auto sym = symtab.lookup(name);
        if (sym && !sym->is_mutable) {
            error("cannot create mutable reference to immutable variable '" + name + "'", line);
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
    if (argList) {
        for (auto& c : argList->children) {
            if (!c->isLeaf) {
                argTypes.push_back(visitExpr(c.get()));
            }
        }
    }

    auto it = functions.find(funcName);
    if (it == functions.end()) {
        error("function '" + funcName + "' not declared", line);
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

    return func->return_type ? func->return_type : SType::makeVoid();
}

STypePtr SemanticAnalyzer::visitIndexExpr(const Node* node) {
    // IndexExpr: Identifier, LBracket, Expr, RBracket
    int line = extractLine(node);
    string varName = extractId(node);

    auto sym = lookupVar(varName, line);
    if (!sym) return SType::makeUnknown();

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

    for (auto& c : node->children) {
        if (!c->isLeaf) {
            elemTypes.push_back(visitExpr(c.get()));
        }
    }

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
        if (nonLeafCount == 0) return SType::makeUnit();
        // Single element without comma => not a tuple (shouldn't reach here, parser handles it)
    }

    vector<STypePtr> elemTypes;
    for (auto& c : node->children) {
        if (!c->isLeaf) {
            elemTypes.push_back(visitExpr(c.get()));
        }
    }

    return SType::makeTuple(move(elemTypes));
}

STypePtr SemanticAnalyzer::visitRangeExpr(const Node* node) {
    // RangeExpr: left, .., right
    for (auto& c : node->children) {
        if (!c->isLeaf) visitExpr(c.get());
    }
    return SType::makeI32(); // ranges are integer
}

STypePtr SemanticAnalyzer::visitParenExpr(const Node* node) {
    // ParenExpr: (, Expr, )
    for (auto& c : node->children) {
        if (!c->isLeaf) return visitExpr(c.get());
    }
    return SType::makeUnknown();
}

STypePtr SemanticAnalyzer::visitAtom(const Node* node) {
    int line = extractLine(node);

    // Identifier node: single leaf child "Identifier: name"
    if (node->label == "Identifier") {
        string name = extractId(node);
        auto sym = lookupVar(name, line);
        if (!sym) return SType::makeUnknown();

        if (!sym->is_assigned) {
            error("variable '" + name + "' used before assignment", line);
        }
        return sym->type;
    }

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
