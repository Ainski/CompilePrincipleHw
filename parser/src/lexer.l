%{
#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>
#include"../include/TokenStream.h"
#include "../include/Token.h"
#include "../include/logprintf.h"

#define YY_DECL int yylex()

// 列号计数器 & Token 起始列号
static int yycolno = 1;
static int token_start_col = 1;

// YY_USER_ACTION: 在每次匹配前记录 Token 的起始列号
#define YY_USER_ACTION token_start_col = yycolno;

int yylex();
void yyerror(const char * msg);

/*=====================================
 * 全局变量
 * file ：输入文件流
 * token ：当前词法单元（用于传递 Token 对象）
 */
std::ifstream INPUT_FILESTREAM;
Token CURRENT_TOKEN;
int POS_COUNT = 1;
// ====================================

#undef YY_INPUT
#ifdef WINDOWS_BUILD
    // Windows: use read() for compatibility with MinGW
    #define YY_INPUT(buf,result,max_size)\
        if (!INPUT_FILESTREAM.eof()) {\
            INPUT_FILESTREAM.read(buf, max_size);\
            result = INPUT_FILESTREAM.gcount();\
            if (result == 0 && INPUT_FILESTREAM.eof()) {\
                result = 0;\
            }\
        } else {\
            result = 0;\
        }
#else
    // Linux: use readsome() for compatibility
    #define YY_INPUT(buf,result,max_size)\
        if (!INPUT_FILESTREAM.eof() && INPUT_FILESTREAM.readsome(buf, max_size) > 0) {\
            result = INPUT_FILESTREAM.gcount();\
        } else {\
            result = 0;\
        }
#endif
%}

%option yylineno
%option noyywrap
DIGIT                           [0-9]
NoneZeroDigit                   [1-9]
OctalDigit                      [0-7]
HexadecimalDigit                [0-9a-fA-F]
BinaryDigit                     [01]
Chars                           [a-zA-Z]
EncodingPrefix                  ("u8"|"u"|"U"|"L")

SimpleEscapeSequence            (\\['"?abfnrtv\\])
OctalEscapeSequence             (\\{OctalDigit}{OctalDigit}?{OctalDigit}?)
HexadecimalEscapeSequence       (\\x{HexadecimalDigit}+)
HexQuad                         ({HexadecimalDigit}{4})
UniversalCharacterName          (\\u{HexQuad}|\\U{HexQuad}{2})
EscapeSequence                  ({SimpleEscapeSequence}|{OctalEscapeSequence}|{HexadecimalEscapeSequence}|{UniversalCharacterName})
SChar                           ([^"\\\r\n]|{EscapeSequence}|\\\n|\\\r\n)
SCharSequence                   {SChar}+
CChar                           [^'\\\r\n]|{EscapeSequence}
CharacterConstant               ("u"|"U"|"L")?\'{CChar}+\'
StringConst                     {EncodingPrefix}?\"{SCharSequence}?\"
UnsignedSuffix                  [uU]
LongSuffix                      [lL]
LongLongSuffix                  (ll|LL)
IntegerSuffix                   (i8|i16|i32|i64|i128|u8|u16|u32|u64|u128|isize|usize)
BinaryConstant                  0[bB]({BinaryDigit}+|{BinaryDigit}({BinaryDigit}\_)*{BinaryDigit})
OctalConstant                   0{OctalDigit}+
HexadecimalConstant             0[xX]({HexadecimalDigit}+|{HexadecimalDigit}({HexadecimalDigit}\_)*{HexadecimalDigit})
DecimalConstant                 ({NoneZeroDigit}{DIGIT}*|{NoneZeroDigit}({DIGIT}|\_)*{DIGIT}|0)
IntegerConstant                 ({DecimalConstant}{IntegerSuffix}?|{HexadecimalConstant}{IntegerSuffix}?|{BinaryConstant})
DigitSequence                   {DIGIT}+
FractionalConstant              ({DigitSequence}?\.{DigitSequence}|{DigitSequence}\.)
ExponentPart                    [eE][+-]?{DigitSequence}
FloatingSuffix                  [fFlL]
DecimalFloatingConstant         {FractionalConstant}{ExponentPart}?{FloatingSuffix}?|{DigitSequence}{ExponentPart}{FloatingSuffix}?
HexadecimalDigitSequence        {HexadecimalDigit}+
HexadecimalFractionalConstant   ({HexadecimalDigitSequence}?\.{HexadecimalDigitSequence}|{HexadecimalDigitSequence}\.)
BinaryExponentPart              [pP][+-]?{DigitSequence}
HexadecimalFloatingConstant     0[xX]({HexadecimalFractionalConstant}|{HexadecimalDigitSequence}){BinaryExponentPart}{FloatingSuffix}?
FloatingConstant                {DecimalFloatingConstant}|{HexadecimalFloatingConstant}
Identifier                      ({Chars}|\_)({Chars}|{DIGIT}|\_)*
LineComment                      "//"(\\\n|\\\r\n|[^\r\n])*
BlockComment                     \/\*([^*]|\*+[^*/])*\*+\/
%%
{LineComment}           { CURRENT_TOKEN = Token(Type::COMMENT, "LineComment", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
{BlockComment}          { CURRENT_TOKEN = Token(Type::COMMENT, "BlockComment", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
{IntegerConstant}       { CURRENT_TOKEN = Token(Type::CONSTANT, "IntegerConstant", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
i32                     { CURRENT_TOKEN = Token(Type::DECLERATOR, "I32", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
let                     { CURRENT_TOKEN = Token(Type::KEYWORD, "Let", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
if                      { CURRENT_TOKEN = Token(Type::KEYWORD, "If", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
else                    { CURRENT_TOKEN = Token(Type::KEYWORD, "Else", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
while                   { CURRENT_TOKEN = Token(Type::KEYWORD, "While", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
return                  { CURRENT_TOKEN = Token(Type::KEYWORD, "Return", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
mut                     { CURRENT_TOKEN = Token(Type::KEYWORD, "Mut", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
fn                      { CURRENT_TOKEN = Token(Type::KEYWORD, "Fn", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
for                     { CURRENT_TOKEN = Token(Type::KEYWORD, "For", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
in                      { CURRENT_TOKEN = Token(Type::KEYWORD, "In", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
loop                    { CURRENT_TOKEN = Token(Type::KEYWORD, "Loop", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
break                   { CURRENT_TOKEN = Token(Type::KEYWORD, "Break", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
continue                { CURRENT_TOKEN = Token(Type::KEYWORD, "Continue", yytext, POS_COUNT++, yylineno, yycolno); return 1; }

{Identifier}            { CURRENT_TOKEN = Token(Type::IDENTIFIER, "Identifier", yytext, POS_COUNT++, yylineno, yycolno); return 1; }

"->"                    { CURRENT_TOKEN = Token(Type::OPERATOR, "Arrow", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
".."                    { CURRENT_TOKEN = Token(Type::OPERATOR, "DotDot", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"."                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Dot", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"=="                    { CURRENT_TOKEN = Token(Type::OPERATOR, "EqualEqual", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"!="                    { CURRENT_TOKEN = Token(Type::OPERATOR, "NotEqual", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
">="                    { CURRENT_TOKEN = Token(Type::OPERATOR, "GreaterEqual", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"<="                    { CURRENT_TOKEN = Token(Type::OPERATOR, "LessEqual", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
">"                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Greater", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"<"                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Less", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"+"                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Plus", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"-"                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Minus", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"*"                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Star", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"/"                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Slash", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"&"                     { CURRENT_TOKEN = Token(Type::OPERATOR, "Ampersand", yytext, POS_COUNT++, yylineno, yycolno); return 1; }

"="                     { CURRENT_TOKEN = Token(Type::DELIMITER, "Assign", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
";"                     { CURRENT_TOKEN = Token(Type::DELIMITER, "Semicolon", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
":"                     { CURRENT_TOKEN = Token(Type::DELIMITER, "Colon", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
","                     { CURRENT_TOKEN = Token(Type::DELIMITER, "Comma", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"("                     { CURRENT_TOKEN = Token(Type::DELIMITER, "LParen", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
")"                     { CURRENT_TOKEN = Token(Type::DELIMITER, "RParen", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"{"                     { CURRENT_TOKEN = Token(Type::DELIMITER, "LBrace", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"}"                     { CURRENT_TOKEN = Token(Type::DELIMITER, "RBrace", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"["                     { CURRENT_TOKEN = Token(Type::DELIMITER, "LBracket", yytext, POS_COUNT++, yylineno, yycolno); return 1; }
"]"                     { CURRENT_TOKEN = Token(Type::DELIMITER, "RBracket", yytext, POS_COUNT++, yylineno, yycolno); return 1; }

"#"                     { CURRENT_TOKEN = Token(Type::DELIMITER, "End", yytext, POS_COUNT++, yylineno, yycolno); return 1; }

[ \t]                   { yycolno += yyleng; }
[\r\n|\r|\n]                  { yycolno = 1; }

.                       { CURRENT_TOKEN = Token(Type::TYPE_EOF, "Unknown", yytext, POS_COUNT++, yylineno, yycolno); return 1; }

%%

TokenStream lex(const string& filepath)
{
    INPUT_FILESTREAM.open(filepath);
    if (!INPUT_FILESTREAM.is_open()) {
        std::cerr << "Failed to open input file." << std::endl;
        exit(1);
    }
    TokenStream ts;
    while(yylex()){
        ts.addToken(CURRENT_TOKEN);
    //    log(LogLevel::INFO , CURRENT_TOKEN.value);
    }
    INPUT_FILESTREAM.close();
    return ts;
}   

void yyerror(const char * msg) {
    std::cerr << "Error: " << msg << std::endl;
}