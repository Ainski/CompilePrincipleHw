#pragma once

// ============================================================
//  TokenStream  –  reads TSV, provides cursor API
// ============================================================
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <cstdio>
#include <cstring>
#include <string.h>
#include <fstream>
#include "../include/config.h"
using namespace std;
// ============================================================
//  Token
// ============================================================
struct Token {
    string type;      // keyword | operator | delimiter | constant | identifier | declerator
    string category;  // Let | Mut | Fn | I32 | Identifier | IntegerConstant | ...
    string value;     // raw text
    int    pos = 0;   // index in token stream (for error messages)
};

static const Token EOF_TOKEN{ "eof", "EOF", "<eof>", -1 };



class TokenStream {
    vector<Token> toks;
    int cur = 0;
    static string trim(const string& s);

public:
    void load(istream& in);
    const Token& peek(int offset = 0) const;
    Token advance();
    Token expect(const string& cat, const string& ctx = "");
    bool check(const string& cat) const;
    bool checkType(const string& tp, int offset = 0) const;
    bool atEnd() const;
    bool match(const string& cat);
    bool addToken(const Token& t,bool IsPrint = false);
    void displayTokens() const;
};

char* rebuild_string(char* str);
