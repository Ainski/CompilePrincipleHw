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
#include "config.h"
#include "Type.h"
#include "Token.h"
using namespace std;

class TokenStream {
protected:
    vector<Token> toks;
    int cur = 0;
    int FILETOKEN = 0 ; 
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
    void to_file(const string& filename) const;
    void remove_comments() ;
};

char* rebuild_string(char* str);
