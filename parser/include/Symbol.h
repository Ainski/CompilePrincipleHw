#ifndef SYMBOL_H
#define SYMBOL_H

#include "SemanticType.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

using namespace std;

struct Symbol {
    string name;
    STypePtr type;
    bool is_mutable;
    bool is_assigned;
    int scope_level;
    int line;

    Symbol(string n, STypePtr t, bool mut, int scope, int ln = 0)
        : name(move(n)), type(t), is_mutable(mut),
          is_assigned(false), scope_level(scope), line(ln) {}
};

struct FunctionInfo {
    string name;
    vector<pair<string, STypePtr>> params; // (name, type)
    STypePtr return_type;
    int line;

    FunctionInfo(string n, vector<pair<string, STypePtr>> p, STypePtr ret, int ln = 0)
        : name(move(n)), params(move(p)), return_type(ret), line(ln) {}
};

class SymbolTable {
    vector<unordered_map<string, shared_ptr<Symbol>>> scopes;
    int current_scope = 0;

public:
    void enterScope();
    void exitScope();
    int scopeLevel() const { return current_scope; }

    void insert(const shared_ptr<Symbol>& sym);
    shared_ptr<Symbol> lookup(const string& name) const;
    shared_ptr<Symbol> lookupCurrent(const string& name) const;
};

#endif
