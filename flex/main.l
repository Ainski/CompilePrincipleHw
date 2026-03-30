%{
#include<stdio.h>
#include<ctype.h>
char* rebuild_string(char* str) {
    int size=strlen(str);
    int i=0;
    char* result=(char*)malloc(size*2+1);
    int result_index=0;
    for(int i=0;i<size;i++){
        if(str[i]=='$'){
            result[result_index++]='$';
            result[result_index++]='$';
        } else if(str[i]=='\n'){
            result[result_index++]='$';
            result[result_index++]='n';
        } else if(str[i]=='\r'){
            result[result_index++]='$';
            result[result_index++]='r';
        } else if(str[i]=='\t'){
            result[result_index++]='$';
            result[result_index++]='t';
        } else if(str[i]=='\0'){
            result[result_index++]='$';
            result[result_index++]='0';
        }
        else if (str[i] == ' '){
            result[result_index++]='$';
            result[result_index++]='_';
        }
        else{
            result[result_index++]=str[i];
        }
    }
    result[result_index]='\0';
    return result;
}

%}
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
IntegerConstant                 ({DecimalConstant}{IntegerSuffix}?|{OctalConstant}{IntegerSuffix}?|{HexadecimalConstant}{IntegerSuffix}?|{BinaryConstant})
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
{LineComment}           { char* res=rebuild_string(yytext); printf("comment\tLineComment\t%s\n", res); free(res); }
{BlockComment}          { char* res=rebuild_string(yytext); printf("comment\tBlockComment\t%s\n", res); free(res); }

i32                     { printf("keyword\tI32\t%s\n", yytext); }
let                     { printf("keyword\tLet\t%s\n", yytext); }
if                      { printf("keyword\tIf\t%s\n", yytext); }
else           { printf("keyword\tElse\t%s\n", yytext); }
while                   { printf("keyword\tWhile\t%s\n", yytext); }
return            { printf("keyword\tReturn\t%s\n", yytext); }
mut                     { printf("keyword\tMut\t%s\n", yytext); }
fn                      { printf("keyword\tFn\t%s\n", yytext); }
for                     { printf("keyword\tFor\t%s\n", yytext); }
in                      { printf("keyword\tIn\t%s\n", yytext); }
loop                    { printf("keyword\tLoop\t%s\n", yytext); }
break                   { printf("keyword\tBreak\t%s\n", yytext); }
continue                { printf("keyword\tContinue\t%s\n", yytext); }

{Identifier}            { printf("identifier\tIdentifier\t%s\n", yytext); }

"->"                    { printf("operator\tArrow\t%s\n", yytext); }
".."                    { printf("operator\tDotDot\t%s\n", yytext); }
"."                     { printf("operator\tDot\t%s\n", yytext); }
"=="                    { printf("operator\tEqualEqual\t%s\n", yytext); }
"!="                    { printf("operator\tNotEqual\t%s\n", yytext); }
">="                    { printf("operator\tGreaterEqual\t%s\n", yytext); }
"<="                    { printf("operator\tLessEqual\t%s\n", yytext); }
">"                     { printf("operator\tGreater\t%s\n", yytext); }
"<"                     { printf("operator\tLess\t%s\n", yytext); }
"+"                     { printf("operator\tPlus\t%s\n", yytext); }
"-"                     { printf("operator\tMinus\t%s\n", yytext); }
"*"                     { printf("operator\tStar\t%s\n", yytext); }
"/"                     { printf("operator\tSlash\t%s\n", yytext); }
"&"                     { printf("operator\tAmpersand\t%s\n", yytext); }

"="                     { printf("delimiter\tAssign\t%s\n", yytext); }
";"                     { printf("delimiter\tSemicolon\t%s\n", yytext); }
":"                     { printf("delimiter\tColon\t%s\n", yytext); }
","                     { printf("delimiter\tComma\t%s\n", yytext); }
"("                     { printf("delimiter\tLParen\t%s\n", yytext); }
")"                     { printf("delimiter\tRParen\t%s\n", yytext); }
"{"                     { printf("delimiter\tLBrace\t%s\n", yytext); }
"}"                     { printf("delimiter\tRBrace\t%s\n", yytext); }
"["                     { printf("delimiter\tLBracket\t%s\n", yytext); }
"]"                     { printf("delimiter\tRBracket\t%s\n", yytext); }

"#"                     { printf("delimiter\tEnd\t%s\n", yytext); }

[ \t\n\r]               ;

.                       { printf("unknown\tUnknown\t%s\n", yytext); }

%%
int main()
{
    printf("TYPE \tCATEGORY \tVALUE \n");
    yylex();
    return 0;
}