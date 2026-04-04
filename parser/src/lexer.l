%{
#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>
#include"../include/config.h"
#include"../include/TokenStream.h"
#include "../include/lexer.h"

#define YY_DECL int yylex()
int yylex();
void yyerror(const char * msg);

/*=====================================
 * 全局变量
 * file ：输入文件流
 * token ：当前词法单元
 */
std::ifstream INPUT_FILESTREAM;
Token CURRENT_TOKEN;
int POS_COUNT = 1;
// ====================================

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size)\
    if (!INPUT_FILESTREAM.eof() && INPUT_FILESTREAM.readsome(buf, max_size) > 0) {\
        result = INPUT_FILESTREAM.gcount();\
    } else {\
        result = 0;\
    }
%}

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
{LineComment}           { CURRENT_TOKEN.type="comment"; CURRENT_TOKEN.category="LineComment"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; } 
{BlockComment}          { CURRENT_TOKEN.type="comment"; CURRENT_TOKEN.category="BlockComment"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
{IntegerConstant}       { CURRENT_TOKEN.type="constant"; CURRENT_TOKEN.category="IntegerConstant"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
i32                     { CURRENT_TOKEN.type="declerator"; CURRENT_TOKEN.category="I32"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
let                     { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Let"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
if                      { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="If"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
else           { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Else"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
while                   { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="While"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
return            { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Return"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
mut                     { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Mut"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
fn                      { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Fn";CURRENT_TOKEN.value=yytext;CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
for                     { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="For";CURRENT_TOKEN.value=yytext;CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
in                      { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="In"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
loop                    { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Loop"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
break                   { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Break"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
continue                { CURRENT_TOKEN.type="keyword"; CURRENT_TOKEN.category="Continue"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }

{Identifier}            { CURRENT_TOKEN.type="identifier"; CURRENT_TOKEN.category="Identifier"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }

"->"                    { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Arrow"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
".."                    { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="DotDot"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"."                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Dot"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"=="                    { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="EqualEqual"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"!="                    { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="NotEqual"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
">="                    { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="GreaterEqual"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"<="                    { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="LessEqual"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
">"                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Greater"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"<"                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Less"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"+"                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Plus"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"-"                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Minus"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"*"                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Star"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"/"                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Slash"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"&"                     { CURRENT_TOKEN.type="operator"; CURRENT_TOKEN.category="Ampersand"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }

"="                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="Assign"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
";"                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="Semicolon"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
":"                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="Colon"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
","                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="Comma"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"("                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="LParen"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
")"                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="RParen"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"{"                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="LBrace"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"}"                     {CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="RBrace"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"["                     {CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="LBracket";CURRENT_TOKEN.value=yytext;CURRENT_TOKEN.pos = POS_COUNT++; return 1; }
"]"                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="RBracket"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }

"#"                     { CURRENT_TOKEN.type="delimiter"; CURRENT_TOKEN.category="End"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }

[ \t\n\r]               ;

.                       { CURRENT_TOKEN.type="unknown"; CURRENT_TOKEN.category="Unknown"; CURRENT_TOKEN.value=yytext; CURRENT_TOKEN.pos = POS_COUNT++; return 1; }

%%

TokenStream ReadLexer(const string& filepath)
{
    printf("TYPE \tCATEGORY \tVALUE \n");
    INPUT_FILESTREAM.open(filepath);
    if (!INPUT_FILESTREAM.is_open()) {
        std::cerr << "Failed to open input file." << std::endl;
        exit(1);
    }
    TokenStream ts;
    while(yylex()){
        ts.addToken(CURRENT_TOKEN,true);
    }
    return ts;
}

void yyerror(const char * msg) {
    std::cerr << "Error: " << msg << std::endl;
}