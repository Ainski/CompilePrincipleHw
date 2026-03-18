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
IntegerSuffix                   ({UnsignedSuffix}{LongSuffix}?|{UnsignedSuffix}{LongLongSuffix}?|{LongSuffix}{UnsignedSuffix}?|{LongLongSuffix}{UnsignedSuffix}?)
BinaryConstant                  0[bB]{BinaryDigit}+
OctalConstant                   0{OctalDigit}+
HexadecimalConstant             0[xX]{HexadecimalDigit}+
DecimalConstant                 ({NoneZeroDigit}{DIGIT}*|0)
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
{StringConst}           { char* res=rebuild_string(yytext); printf("constant\tStringConst\t%s\n", res); free(res); }
{CharacterConstant}     { printf("constant\tCharacterConst\t%s\n", yytext); }
{FloatingConstant}      { printf("constant\tFloatingConst\t%s\n", yytext); }
{IntegerConstant}       { printf("constant\tIntegerConst\t%s\n", yytext); }
break           { printf("keyword\tBreak\t%s\n", yytext); }
case            { printf("keyword\tCase\t%s\n", yytext); }
continue        { printf("keyword\tContinue\t%s\n", yytext); }
default         { printf("keyword\tDefault\t%s\n", yytext); }
do              { printf("keyword\tDo\t%s\n", yytext); }
else            { printf("keyword\tElse\t%s\n", yytext); }
for             { printf("keyword\tFor\t%s\n", yytext); }
if              { printf("keyword\tIf\t%s\n", yytext); }
return          { printf("keyword\tReturn\t%s\n", yytext); }
switch          { printf("keyword\tSwitch\t%s\n", yytext); }
while           { printf("keyword\tWhile\t%s\n", yytext); }
_Alignas        { printf("keyword\tUAlignas\t%s\n", yytext); }
_Generic        { printf("keyword\tUGeneric\t%s\n", yytext); }
_Static_assert  { printf("keyword\tUStaticAssert\t%s\n", yytext); }
__builtin_va_arg { printf("keyword\tUBVA\t%s\n", yytext); }
__builtin_offsetof { printf("keyword\tUBO\t%s\n", yytext); }
asm             { printf("keyword\tUAsm\t%s\n", yytext); }
fortran         { printf("keyword\tUFortran\t%s\n", yytext); }
__asm           { printf("keyword\tUAsm\t%s\n", yytext); }
__asm__         { printf("keyword\tUAsm\t%s\n", yytext); }

"->"            { printf("operator\tArrow\t%s\n", yytext); }
"++"            { printf("operator\tInc\t%s\n", yytext); }
"--"            { printf("operator\tDec\t%s\n", yytext); }
"<<="           { printf("operator\tLShiftAssign\t%s\n", yytext); }
">>="           { printf("operator\tRShiftAssign\t%s\n", yytext); }
"+="            { printf("operator\tPlusAssign\t%s\n", yytext); }
"-="            { printf("operator\tMinusAssign\t%s\n", yytext); }
"*="            { printf("operator\tStarAssign\t%s\n", yytext); }
"/="            { printf("operator\tSlashAssign\t%s\n", yytext); }
"%="            { printf("operator\tPercentAssign\t%s\n", yytext); }
"&="            { printf("operator\tAmpAssign\t%s\n", yytext); }
"|="            { printf("operator\tBiterAssign\t%s\n", yytext); }
"^="            { printf("operator\tCaretAssign\t%s\n", yytext); }
"=="            { printf("operator\tEq\t%s\n", yytext); }
"!="            { printf("operator\tNe\t%s\n", yytext); }
"<="            { printf("operator\tLe\t%s\n", yytext); }
">="            { printf("operator\tGe\t%s\n", yytext); }
"<<"            { printf("operator\tLShift\t%s\n", yytext); }
">>"            { printf("operator\tRShift\t%s\n", yytext); }
"&&"            { printf("operator\tAnd\t%s\n", yytext); }
"||"            { printf("operator\tOr\t%s\n", yytext); }

\(              { printf("separator\tLParen\t%s\n", yytext); }
\)              { printf("separator\tRParen\t%s\n", yytext); }
\{              { printf("separator\tLBrace\t%s\n", yytext); }
\}              { printf("separator\tRBrace\t%s\n", yytext); }
\[              { printf("separator\tLBracket\t%s\n", yytext); }
\]              { printf("separator\tRBracket\t%s\n", yytext); }
,               { printf("separator\tComma\t%s\n", yytext); }
\.              { printf("separator\tDot\t%s\n", yytext); }

\+              { printf("operator\tPos\t%s\n", yytext); }
\-              { printf("operator\tneg\t%s\n", yytext); }
\*              { printf("operator\tStar\t%s\n", yytext); }
\/              { printf("operator\tSlash\t%s\n", yytext); }
\%              { printf("operator\tPercent\t%s\n", yytext); }
\&              { printf("operator\tAmp\t%s\n", yytext); }
\|              { printf("operator\tBiter\t%s\n", yytext); }
\^              { printf("operator\tCaret\t%s\n", yytext); }
!               { printf("operator\tExclam\t%s\n", yytext); }
\?              { printf("operator\tQuestion\t%s\n", yytext); }
\:              { printf("operator\tColon\t%s\n", yytext); }
"="             { printf("operator\tAssign\t%s\n", yytext); }
"<"             { printf("operator\tLt\t%s\n", yytext); }
">"             { printf("operator\tGt\t%s\n", yytext); }
\#              { printf("operator\tHash\t%s\n", yytext); }
"'"             { printf("operator\tSingleQuote\t%s\n", yytext); }
"\""            { printf("operator\tDoubleQuote\t%s\n", yytext); }

";"             { printf("delimiter\tDelimiter\t%s\n", yytext); }

signed          { printf("declarator\tSigned\t%s\n", yytext); }
unsigned        { printf("declarator\tUnsigned\t%s\n", yytext); }

auto            { printf("declarator\tAuto\t%s\n", yytext); }
static          { printf("declarator\tStatic\t%s\n", yytext); }
extern          { printf("declarator\tExtern\t%s\n", yytext); }
register        { printf("declarator\tRegister\t%s\n", yytext); }
_Thread_local   { printf("declarator\tUThreadULocal\t%s\n", yytext); }
typedef         { printf("declarator\tTypedef\t%s\n", yytext); }

void            { printf("declarator\tVoid\t%s\n", yytext); }
char            { printf("declarator\tChar\t%s\n", yytext); }
short           { printf("declarator\tShort\t%s\n", yytext); }
int             { printf("declarator\tInt\t%s\n", yytext); }
long            { printf("declarator\tLong\t%s\n", yytext); }
float           { printf("declarator\tFloat\t%s\n", yytext); }
double          { printf("declarator\tDouble\t%s\n", yytext); }
_Bool           { printf("declarator\tUBool\t%s\n", yytext); }
_Complex        { printf("declarator\tUComplex\t%s\n", yytext); }
_Imaginary      { printf("declarator\tUImaginary\t%s\n", yytext); }
__m128          { printf("declarator\tUM128\t%s\n", yytext); }
__m128d         { printf("declarator\tUM128d\t%s\n", yytext); }
__m128i         { printf("declarator\tUM128i\t%s\n", yytext); }

struct          { printf("declarator\tStruct\t%s\n", yytext); }
union           { printf("declarator\tUnion\t%s\n", yytext); }
enum            { printf("declarator\tEnum\t%s\n", yytext); }

const           { printf("declarator\tConst\t%s\n", yytext); }
restrict        { printf("declarator\tRestrict\t%s\n", yytext); }
volatile        { printf("declarator\tVolatile\t%s\n", yytext); }
__volatile__    { printf("declarator\tUVolatile\t%s\n", yytext); }
_Atomic         { printf("declarator\tUAtomic\t%s\n", yytext); }

inline          { printf("declarator\tInline\t%s\n", yytext); }
__inline__      { printf("declarator\tUInline\t%s\n", yytext); }
_Noreturn       { printf("declarator\tUNoreturn\t%s\n", yytext); }
__stdcall       { printf("declarator\tUStdcall\t%s\n", yytext); }
__cdecl         { printf("declarator\tUCdecl\t%s\n", yytext); }
__clrcall       { printf("declarator\tUClrcall\t%s\n", yytext); }
__fastcall      { printf("declarator\tUFastcall\t%s\n", yytext); }
__thiscall      { printf("declarator\tUThiscall\t%s\n", yytext); }
__vectorcall    { printf("declarator\tUVectorcall\t%s\n", yytext); }

__typeof__      { printf("declarator\tUTypeof\t%s\n", yytext); }
__extension__   { printf("declarator\tUExtension\t%s\n", yytext); }
__declspec      { printf("declarator\tUDeclspec\t%s\n", yytext); }
__attribute__   { printf("declarator\tUAttribute\t%s\n", yytext); }

{Identifier}    { printf("identifier\tIdentifier\t%s\n", yytext); }

[ \t\n\r]       ;

.               { printf("unknown\tUnknown\t%s\n", yytext); }

%%
int main()
{
    printf("TYPE\tCATEGORY\tVALUE\n");
    yylex();
    return 0;
}