grammar C;

primaryExpression : IDENTIFIER ;
// Comments :

LINE_COMMENT
    : '//' ~[\r\n]* -> skip
    ;

BLOCK_COMMENT
    : '/*' .*? '*/' -> skip
    ;

// Characters

DIGIT : [0-9] ; // Any digit

OCTAL_DIGIT : [0-7] ; // Any octal digit

HEX_DIGIT : [0-9a-fA-F] ; // Any hexadecimal digit

BINARY_DIGIT : [0-1] ;// Any binary digit

POSITIVE_DIGIT : [1-9] ; // Any positive digit (1-9)

WHITE_SPACE : [ \t\r\n]+ -> skip ; // Skip whitespace

NEWLINE : [\r\n]+ ; // Newline characters

CHARS : [a-zA-Z] ; // Any letter (for identifiers)

UPCHARS : [A-Z] ; // Uppercase letters

LOWERCHARS : [a-z] ; // Lowercase letters

// PUNCATIONS  符号 https://en.cppreference.com/w/c/language/punctuators.html
ARROW            : '->' ;
INC              : '++' ;
DEC              : '--' ;
LSHIFT_ASSIGN    : '<<=' ;
RSHIFT_ASSIGN    : '>>=' ;
PLUS_ASSIGN      : '+=' ;
MINUS_ASSIGN     : '-=' ;
STAR_ASSIGN      : '*=' ;
SLASH_ASSIGN     : '/=' ;
PERCENT_ASSIGN   : '%=' ;
AMP_ASSIGN       : '&=' ;
BITOR_ASSIGN     : '|=' ;
CARET_ASSIGN     : '^=' ;
EQ               : '==' ;
NE               : '!=' ;
LE               : '<=' ;
GE               : '>=' ;
LSHIFT           : '<<' ;
RSHIFT           : '>>' ;
AND              : '&&' ;
OR               : '||' ;

LPAREN           : '(' ;
RPAREN           : ')' ;
LBRACK           : '[' ;
RBRACK           : ']' ;
LBRACE           : '{' ;
RBRACE           : '}' ;
SEMI             : ';' ;
COMMA            : ',' ;
DOT              : '.' ;
POS              : '+' ;   // 正号/加号
NEG              : '-' ;   // 负号/减号
STAR             : '*' ;   // 乘号/解引用
SLASH            : '/' ;   // 除号
PERCENT          : '%' ;   // 取模
AMP              : '&' ;   // 按位与/取地址
BITOR            : '|' ;   // 按位或
CARET            : '^' ;   // 按位异或
TILDE            : '~' ;   // 按位取反
EXCLAM           : '!' ;   // 逻辑非
QUESTION         : '?' ;   // 条件运算符的问号
COLON            : ':' ;   // 条件运算符的冒号
ASSIGN           : '=' ;   // 赋值
LT               : '<' ;   // 小于
GT               : '>' ;   // 大于
UNDERSCORE       : '_' ;   // 下划线
HASH             : '#' ;   // 预处理指令的井号


ALL_CHAR : . ; // Any character (including newlines)

/*标识符https://en.cppreference.com/w/c/language/identifiers.html*/
IDENTIFIER
    : (UNDERSCORE | CHARS) (UNDERSCORE | CHARS | DIGIT)*
    ; // 识别标识符：以字母或下划线开头，后续可以包含字母、数字或下划线


// keywords https://en.cppreference.com/w/c/keyword.html


// === 标准关键字 (按C23标准整理，包含各版本) ===

// C89/C90 标准关键字
AUTO            : 'auto' ;
BREAK           : 'break' ;
CASE            : 'case' ;
CHAR            : 'char' ;
CONST           : 'const' ;
CONTINUE        : 'continue' ;
DEFAULT         : 'default' ;
DO              : 'do' ;
DOUBLE          : 'double' ;
ELSE            : 'else' ;
ENUM            : 'enum' ;
EXTERN          : 'extern' ;
FLOAT           : 'float' ;
FOR             : 'for' ;
GOTO            : 'goto' ;
IF              : 'if' ;
INT             : 'int' ;
LONG            : 'long' ;
REGISTER        : 'register' ;
RETURN          : 'return' ;
SHORT           : 'short' ;
SIGNED          : 'signed' ;
SIZEOF          : 'sizeof' ;
STATIC          : 'static' ;
STRUCT          : 'struct' ;
SWITCH          : 'switch' ;
TYPEDEF         : 'typedef' ;
UNION           : 'union' ;
UNSIGNED        : 'unsigned' ;
VOID            : 'void' ;
VOLATILE        : 'volatile' ;
WHILE           : 'while' ;

// C99 新增关键字
INLINE          : 'inline' ;
RESTRICT        : 'restrict' ;
U_BOOL           : '_Bool' ;          // C99, C23中弃用但保留
U_COMPLEX        : '_Complex' ;        // C99
U_IMAGINARY      : '_Imaginary' ;      // C99

// C11 新增关键字
U_ALIGNAS        : '_Alignas' ;        // C11, C23中弃用
U_ALIGNOF        : '_Alignof' ;        // C11, C23中弃用
U_ATOMIC         : '_Atomic' ;         // C11
U_GENERIC        : '_Generic' ;        // C11
U_NORETURN       : '_Noreturn' ;       // C11, C23中弃用
U_STATIC_ASSERT  : '_Static_assert' ;  // C11, C23中弃用
U_THREAD_LOCAL   : '_Thread_local' ;   // C11, C23中弃用

// C23 新增/升级为关键字
ALIGNAS         : 'alignas' ;         // C23，替代 _Alignas
ALIGNOF         : 'alignof' ;         // C23，替代 _Alignof
BOOL            : 'bool' ;            // C23，替代 _Bool
STATIC_ASSERT   : 'static_assert' ;   // C23，替代 _Static_assert
THREAD_LOCAL    : 'thread_local' ;    // C23，替代 _Thread_local
U_BITINT         : '_BitInt' ;         // C23
U_DECIMAL128     : '_Decimal128' ;     // C23
U_DECIMAL32      : '_Decimal32' ;      // C23
U_DECIMAL64      : '_Decimal64' ;      // C23
CONSTEXPR       : 'constexpr' ;       // C23 (若未来加入，此处预留)

// === 预处理器运算符与特殊关键字 ===
// 这些通常在预处理上下文中识别，但为了完整性可列出

// 预处理器指令关键字（通常由预处理器处理，但词法分析可识别）
DEFINE          : 'define' ;          // #define 的一部分
IF_DEF          : 'ifdef' ;           // #ifdef
IF_NDEF         : 'ifndef' ;          // #ifndef
ELIF            : 'elif' ;
ENDIF           : 'endif' ;
INCLUDE         : 'include' ;
LINE            : 'line' ;
ERROR           : 'error' ;
PRAGMA          : 'pragma' ;
UNDEF           : 'undef' ;
//IF              : 'if' ;              // 已在标准关键字中，但预处理器也使用

// 预处理器运算符 (C99及以后)
U_PRAGMA         : '_Pragma' ;          // C99

// 扩展关键字 (条件支持)
ASM             : 'asm' ;              // 常见扩展
FORTRA          : 'fortran' ;          // 扩展，罕见

// === 注意：true/false 在C中是宏，不是关键字 (C23前) ===
// C23起 true/false 成为关键字？网页未明确，但通常仍为宏。建议不列为关键字。

