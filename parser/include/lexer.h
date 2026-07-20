#ifndef LEXER_H
#define LEXER_H
// yylex 接口定义
#include "TokenStream.h"
TokenStream lex(const string& filepath);
TokenStream stringlexer(const string& input);
#endif