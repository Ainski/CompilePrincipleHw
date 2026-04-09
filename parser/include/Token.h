#ifndef TOKEN_H
#define TOKEN_H
#include "Type.h"
#include <string>
using namespace std;

class Token {
public:
    Type type;         // keyword | operator | delimiter | constant | identifier | declerator
    string category;   // Let | Mut | Fn | I32 | Identifier | IntegerConstant | ...
    string value;      // raw text
    int    pos ;    // index in token stream (for error messages)
    int    lineno ; // line number in source file (for error messages)
    int    colno ;  // column number in source file (for error messages)

    Token() = default;
    Token(Type t, const string& cat, const string& val, int p = 0, int ln = 0, int cn = 0)
        : type(t), category(cat), value(val), pos(p), lineno(ln), colno(cn) {}
    friend ostream& operator<<(ostream& os, const Token& t) {
        os << t.type << "\t" << t.category << "\t" << t.value << "\t" << t.pos << "\t" << t.lineno << "\t" << t.colno;
        return os;
    }
    
};


static const Token EOF_TOKEN { Type::TYPE_EOF, "EOF", "<eof>", -1 };

#endif