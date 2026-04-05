#include "Type.h"

string typeToString(const Type& t) {
    switch (t) {
        case Type::KEYWORD: return "keyword";
        case Type::OPERATOR: return "operator";
        case Type::DELIMITER: return "delimiter";
        case Type::CONSTANT: return "constant";
        case Type::IDENTIFIER: return "identifier";
        case Type::DECLERATOR: return "declerator";
        case Type::COMMENT: return "comment";
        case Type::TYPE_EOF: return "eof";
        default: return "unknown";
    }
}

ostream &operator<<(ostream &os, const Type &t) {
    os << typeToString(t);
    return os;
}
Type StringToType(const string& s){
    if (s == "keyword") return Type::KEYWORD;
    if (s == "operator") return Type::OPERATOR;
    if (s == "delimiter") return Type::DELIMITER;
    if (s == "constant") return Type::CONSTANT;
    if (s == "identifier") return Type::IDENTIFIER;
    if (s == "declerator") return Type::DECLERATOR;
    if (s == "comment") return Type::COMMENT;
    if (s == "eof") return Type::TYPE_EOF;
    throw runtime_error("Unknown type string: " + s);

}
