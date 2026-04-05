#ifndef TYPE_H
#define TYPE_H
#include <string>
#include <stdexcept>
#include <iostream>
using namespace std;
// ============================================================
//  Token
// ============================================================
enum class Type {
    KEYWORD,
    OPERATOR,
    DELIMITER,
    CONSTANT,
    IDENTIFIER,
    DECLERATOR,
    COMMENT,
    TYPE_EOF
};

string typeToString(const Type& t);
ostream& operator<<(ostream& os, const Type& t);
Type StringToType(const string& s);

#endif