#include "../include/Symbol.h"

void SymbolTable::enterScope() {
    current_scope++;
    scopes.emplace_back();
}

void SymbolTable::exitScope() {
    if (!scopes.empty()) scopes.pop_back();
    if (current_scope > 0) current_scope--;
}

void SymbolTable::insert(const shared_ptr<Symbol>& sym) {
    if (scopes.empty()) scopes.emplace_back();
    scopes.back()[sym->name] = sym;
}

shared_ptr<Symbol> SymbolTable::lookup(const string& name) const {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return it->second;
    }
    return nullptr;
}

shared_ptr<Symbol> SymbolTable::lookupCurrent(const string& name) const {
    if (scopes.empty()) return nullptr;
    auto it = scopes.back().find(name);
    if (it != scopes.back().end()) return it->second;
    return nullptr;
}
