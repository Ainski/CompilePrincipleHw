grammar C;

primaryexpression : varName ;

varName:(Chars|Underscore)(Underscore|Chars|Digit)* ; // 变量名，直接使用标识符即可


expression
    : constExpression // 用户仅仅写一个神秘的常量 不加任何说明 这是被允许的。例如 3+6 ;
    | arithExpression // 经过一次或者多次运算得到的表达式
    ;


/*
关于是否应该采取constExpression（不可运算表达式）和arithExpression（可运算表达式）

在以下情况当中会有区别：
- 数组访问 (a+6)[i] 是被允许的 (6+6)[i] 是不被允许的
- 6 = 3 是不被允许的 a = 3 是被允许的
两者的区别在于：
前者仅仅携带了一个值，而后者携带了一个存储单元
因此，两者做了区分。
*/
constExpression      // 运算结果不可变的 表达式 todo: 这里的定义还不够完善，后续需要根据实际情况进行调整
    : StringConst
    | CharacterConstant
    | IntegerConstant
    | FloatingConstant
    ;
arithExpression // 参考 https://c.biancheng.net/view/oblaq24.html
// 运算结果可变的表达式 todo
    /* 第一组*/
    : varName
    | arithExpression LBrack expression RBrack // 数组访问
    | LParen arithExpression? RParen // 优先运算
    | arithExpression LParen argumentExpressionList ? RParen // 第一个 arithExpression 是允许数组访问的到的 函数指针
    | arithExpression Dot Identifier // 结构体成员访问
    | arithExpression Arrow Identifier // 结构体指针成员访问
    /* 第二组*/
    | Neg arithExpression; // 一元负号



// ======================== 类型名称 ========================
typeName
    : specifierQualifierList abstractDeclarator? // 例如 int * 后面的 * 就是 abstractDeclarator
    ;
specifierQualifierList
    : (typeSpecifier | typeQualifier) specifierQualifierList?
    ;
typeSpecifier
    : 'void'
    | 'char'
    | 'short'
    | 'int'
    | 'long'
    | 'float'
    | 'double'
    | 'signed'
    | 'unsigned'
    | '_Bool'
    | '_Complex'
    | '__m128'
    | '__m128d'
    | '__m128i'
    | '__extension__' '(' ('__m128' | '__m128d' | '__m128i') ')'
    | atomicTypeSpecifier
    | structOrUnionSpecifier
    | enumSpecifier
    | typedefName
    | '__typeof__' '(' expression ')' // GCC extension
    ;
atomicTypeSpecifier
    : '_Atomic' '(' typeName ')'
    ;
structOrUnionSpecifier
    : 'struct' ; // Todo: 完善结构体和联合体的定义
enumSpecifier
    : 'enum' ; // Todo: 完善枚举类型的定义
typedefName
    : Identifier ;

typeQualifier
    : 'const'
    | 'restrict'
    | 'volatile'
    | '_Atomic'
    ;

abstractDeclarator
    : pointer
    | pointer? directAbstractDeclarator gccDeclaratorExtension*
    ;

directAbstractDeclarator
    : '(' abstractDeclarator ')' gccDeclaratorExtension*
    | '[' typeQualifierList? expression? ']'
    | '[' 'static' typeQualifierList? expression ']'
    | '[' typeQualifierList 'static' expression ']'
    | '[' '*' ']'
    | '(' parameterTypeList? ')' gccDeclaratorExtension*
    | directAbstractDeclarator '[' typeQualifierList? expression? ']'
    | directAbstractDeclarator '[' 'static' typeQualifierList? expression ']'
    | directAbstractDeclarator '[' typeQualifierList 'static' expression ']'
    | directAbstractDeclarator '[' '*' ']'
    | directAbstractDeclarator '(' parameterTypeList? ')' gccDeclaratorExtension*
    ;
pointer
    : (('*' | '^') typeQualifierList?)+ // ^ - Blocks language extension
    ;
typeQualifierList
    : typeQualifier+
    ;
gccDeclaratorExtension
    : '__asm' '(' StringConst+ ')'
    | gccAttributeSpecifier
    ;
gccAttributeSpecifier
    : '__attribute__' '(' '(' gccAttributeList ')' ')'
    ;
gccAttributeList
    : gccAttribute? (',' gccAttribute?)*
    ;
gccAttribute
    : ~(',' | '(' | ')') // relaxed def for "identifier or reserved word"
    ('(' argumentExpressionList? ')')?
    ;
argumentExpressionList
    : expression (',' expression)*
    ;
parameterTypeList
    : parameterList (',' '...')?
    ;
parameterList
    : parameterDeclaration (',' parameterDeclaration)*
    ;
parameterDeclaration : 'todo' ; // todo 变量声明不在此处完成，此处仅仅为了避免报错
// ======================== Const statements ========================
StringConst : EncodingPrefix? DoubleQuote SCharSequence? DoubleQuote ;

/*fragment*/ EncodingPrefix : 'u8' | 'u' | 'U' | 'L' ;

/*fragment*/ SCharSequence
    : SChar+
    ;

/*fragment*/ SChar
    : ~["\\\r\n]
    | EscapeSequence // 转义字符
    | '\\\n'   // 接受 续行符
    | '\\\r\n' // 接受 续行符
    ;

/*fragment*/ EscapeSequence
    : SimpleEscapeSequence
    | OctalEscapeSequence
    | HexadecimalEscapeSequence
    | UniversalCharacterName
    ;

/*fragment*/ SimpleEscapeSequence
    : '\\' ['"?abfnrtv\\]
    ;// \a \r \n 等转义字符

/*fragment*/ OctalEscapeSequence
    : '\\' OctalDigit OctalDigit? OctalDigit?
    ;// 八进制转义字符，最多三位

/*fragment*/ HexadecimalEscapeSequence
    : '\\x' HexadecimalDigit+
    ;// 十六进制转义字符，至少一位

/*fragment*/ UniversalCharacterName
    : '\\u' HexQuad
    | '\\U' HexQuad HexQuad
    ;// Unicode 转义字符，\u 后跟 4 位十六进制，\U 后跟 8 位十六进制

/*fragment*/ HexQuad
    : HexadecimalDigit HexadecimalDigit HexadecimalDigit HexadecimalDigit
    ;

constant
    : IntegerConstant
    | FloatingConstant
    //| EnumerationConstant
    | CharacterConstant
    ;
CharacterConstant
    : '\'' CCharSequence '\''
    | 'L\'' CCharSequence '\''
    | 'u\'' CCharSequence '\''
    | 'U\'' CCharSequence '\''
    ;
/*fragment*/ CCharSequence
    : CChar+
    ;
/*fragment*/ CChar
    : ~['\\\r\n]
    | EscapeSequence
    ;
IntegerConstant
    : DecimalConstant IntegerSuffix?
    | OctalConstant IntegerSuffix?
    | HexadecimalConstant IntegerSuffix?
    | BinaryConstant
    ;
/*fragment*/ IntegerSuffix
    : UnsignedSuffix LongSuffix?
    | UnsignedSuffix LongLongSuffix
    | LongSuffix UnsignedSuffix?
    | LongLongSuffix UnsignedSuffix?
    ;
/*fragment*/ UnsignedSuffix
    : [uU]
    ;

/*fragment*/ LongSuffix
    : [lL]
    ;

/*fragment*/ LongLongSuffix
    : 'll'
    | 'LL'
    ;

/*fragment*/ BinaryConstant
    : '0' [bB] [0-1]+
    ;

/*fragment*/ DecimalConstant
    : NonzeroDigit Digit*
    ;

/*fragment*/ OctalConstant
    : '0' OctalDigit*
    ;

/*fragment*/ HexadecimalConstant
    : HexadecimalPrefix HexadecimalDigit+
    ;

/*fragment*/ HexadecimalPrefix
    : '0' [xX]
    ;

 FloatingConstant
    : DecimalFloatingConstant
    | HexadecimalFloatingConstant
    ;
/*fragment*/ DecimalFloatingConstant
    : FractionalConstant ExponentPart? FloatingSuffix?
    | DigitSequence ExponentPart FloatingSuffix?
    ;
/*fragment*/ ExponentPart
    : [eE] Sign? DigitSequence
    ;
/*fragment*/ Sign : Pos | Neg ;

DigitSequence
    : Digit+
    ;

/*fragment*/ FractionalConstant
    : DigitSequence? '.' DigitSequence
    | DigitSequence '.'
    ;

/*fragment*/ FloatingSuffix
    : [flFL]
    ;

/*fragment*/ HexadecimalFloatingConstant
    : HexadecimalPrefix (HexadecimalFractionalConstant | HexadecimalDigitSequence) BinaryExponentPart FloatingSuffix?
    ;

/*fragment*/ HexadecimalFractionalConstant
    : HexadecimalDigitSequence? '.' HexadecimalDigitSequence
    | HexadecimalDigitSequence '.'
    ;
/*fragment*/ HexadecimalDigitSequence
    : HexadecimalDigit+
    ;

/*fragment*/ BinaryExponentPart
    : [pP] Sign? DigitSequence
    ;
/*======================== 标识符 ========================https://en.cppreference.com/w/c/language/identifiers.html*/
Identifier
    : (Underscore | Chars) (Underscore | Chars | Digit)*
    ; // 识别标识符：以字母或下划线开头，后续可以包含字母、数字或下划线


// keywords https://en.cppreference.com/w/c/keyword.html
// =================================Comments=================================
LineComment
    : '//' ~[\r\n]* -> skip
    ;

BlockComment
    : '/*' .*? '*/' -> skip
    ;
// ======================== Characters ========================

/*fragment*/ Digit : [0-9] ; // Any digit

/*fragment*/ NonzeroDigit : [1-9] ; // Any non-zero digit

/*fragment*/ OctalDigit : [0-7] ; // Any octal digit

/*fragment*/ HexadecimalDigit : [0-9a-fA-F] ; // Any hexadecimal digit

/*fragment*/ BinaryDigit : [0-1] ;// Any binary digit

/*fragment*/ PositiveDigit : [1-9] ; // Any positive digit (1-9)

WhiteSpace : [ \t\r\n]+ -> skip ; // Skip whitespace

/*fragment*/ Newline : [\r\n]+ ; // Newline characters

/*fragment*/ Chars : [a-zA-Z] ; // Any letter (for identifiers)

/*fragment*/ UpChars : [A-Z] ; // Uppercase letters

/*fragment*/ LowerChars : [a-z] ; // Lowercase letters

// ======================== PUNCATIONS  符号 ======================== https://en.cppreference.com/w/c/language/punctuators.html
/*fragment*/ Arrow            : '->' ;
/*fragment*/ Inc              : '++' ;
/*fragment*/ Dec              : '--' ;
/*fragment*/ LShiftAssign     : '<<=' ;
/*fragment*/ RShiftAssign     : '>>=' ;
/*fragment*/ PlusAssign       : '+=' ;
/*fragment*/ MinusAssign      : '-=' ;
/*fragment*/ StarAssign       : '*=' ;
/*fragment*/ SlashAssign      : '/=' ;
/*fragment*/ PercentAssign    : '%=' ;
/*fragment*/ AmpAssign        : '&=' ;
/*fragment*/ BitorAssign      : '|=' ;
/*fragment*/ CaretAssign      : '^=' ;
/*fragment*/ Eq               : '==' ;
/*fragment*/ Ne               : '!=' ;
/*fragment*/ Le               : '<=' ;
/*fragment*/ Ge               : '>=' ;
/*fragment*/ LShift           : '<<' ;
/*fragment*/ RShift           : '>>' ;
/*fragment*/ And              : '&&' ;
/*fragment*/ Or               : '||' ;

/*fragment*/ LParen           : '(' ;
/*fragment*/ RParen           : ')' ;
/*fragment*/ LBrack           : '[' ;
/*fragment*/ RBrack           : ']' ;
/*fragment*/ LBrace           : '{' ;
/*fragment*/ RBrace           : '}' ;
/*fragment*/ Semi             : ';' ;
/*fragment*/ Comma            : ',' ;
/*fragment*/ Dot              : '.' ;
/*fragment*/ Pos              : '+' ;   // 正号/加号
/*fragment*/ Neg              : '-' ;   // 负号/减号
/*fragment*/ Star             : '*' ;   // 乘号/解引用
/*fragment*/ Slash            : '/' ;   // 除号
/*fragment*/ Percent          : '%' ;   // 取模
/*fragment*/ Amp              : '&' ;   // 按位与/取地址
/*fragment*/ Bitor            : '|' ;   // 按位或
/*fragment*/ Caret            : '^' ;   // 按位异或
/*fragment*/ Tilde            : '~' ;   // 按位取反
/*fragment*/ Exclam           : '!' ;   // 逻辑非
/*fragment*/ Question         : '?' ;   // 条件运算符的问号
/*fragment*/ Colon            : ':' ;   // 条件运算符的冒号
/*fragment*/ Assign           : '=' ;   // 赋值
/*fragment*/ Lt               : '<' ;   // 小于
/*fragment*/ Gt               : '>' ;   // 大于
/*fragment*/ Underscore       : '_' ;   // 下划线
/*fragment*/ Hash             : '#' ;   // 预处理指令的井号
/*fragment*/ SingleQuote      : '\'' ;  // 单引号字符
/*fragment*/ DoubleQuote      : '"' ;  // 双引号字符

/*fragment*/ AllChar : . ; // Any character (including newlines)


// === 标准关键字 (按 C23 标准整理，包含各版本) ===

// C89/C90 标准关键字
/*fragment*/ Auto            : 'auto' ;
/*fragment*/ Break           : 'break' ;
/*fragment*/ Case            : 'case' ;
/*fragment*/ Char            : 'char' ;
/*fragment*/ Const           : 'const' ;
/*fragment*/ Continue        : 'continue' ;
/*fragment*/ Default         : 'default' ;
/*fragment*/ Do              : 'do' ;
/*fragment*/ Double          : 'double' ;
/*fragment*/ Else            : 'else' ;
/*fragment*/ Enum            : 'enum' ;
/*fragment*/ Extern          : 'extern' ;
/*fragment*/ Float           : 'float' ;
/*fragment*/ For             : 'for' ;
/*fragment*/ Goto            : 'goto' ;
/*fragment*/ If              : 'if' ;
/*fragment*/ Int             : 'int' ;
/*fragment*/ Long            : 'long' ;
/*fragment*/ Register        : 'register' ;
/*fragment*/ Return          : 'return' ;
/*fragment*/ Short           : 'short' ;
/*fragment*/ Signed          : 'signed' ;
/*fragment*/ Sizeof          : 'sizeof' ;
/*fragment*/ Static          : 'static' ;
/*fragment*/ Struct          : 'struct' ;
/*fragment*/ Switch          : 'switch' ;
/*fragment*/ Typedef         : 'typedef' ;
/*fragment*/ Union           : 'union' ;
/*fragment*/ Unsigned        : 'unsigned' ;
/*fragment*/ Void            : 'void' ;
/*fragment*/ Volatile        : 'volatile' ;
/*fragment*/ While           : 'while' ;

// C99 新增关键字
/*fragment*/ Inline          : 'inline' ;
/*fragment*/ Restrict        : 'restrict' ;
/*fragment*/ UBool           : '_Bool' ;          // C99, C23 中弃用但保留
/*fragment*/ UComplex        : '_Complex' ;        // C99
/*fragment*/ UImaginary      : '_Imaginary' ;      // C99

// C11 新增关键字
/*fragment*/ UAlignas        : '_Alignas' ;        // C11, C23 中弃用
/*fragment*/ UAlignof        : '_Alignof' ;        // C11, C23 中弃用
/*fragment*/ UAtomic         : '_Atomic' ;         // C11
/*fragment*/ UGeneric        : '_Generic' ;        // C11
/*fragment*/ UNoreturn       : '_Noreturn' ;       // C11, C23 中弃用
/*fragment*/ UStaticAssert   : '_Static_assert' ;  // C11, C23 中弃用
/*fragment*/ UThreadLocal    : '_Thread_local' ;   // C11, C23 中弃用

// C23 新增/升级为关键字
/*fragment*/ Alignas         : 'alignas' ;         // C23，替代 _Alignas
/*fragment*/ Alignof         : 'alignof' ;         // C23，替代 _Alignof
/*fragment*/ Bool            : 'bool' ;            // C23，替代 _Bool
/*fragment*/ StaticAssert    : 'static_assert' ;   // C23，替代 _Static_assert
/*fragment*/ ThreadLocal     : 'thread_local' ;    // C23，替代 _Thread_local
/*fragment*/ UBitInt         : '_BitInt' ;         // C23
/*fragment*/ UDecimal128     : '_Decimal128' ;     // C23
/*fragment*/ UDecimal32      : '_Decimal32' ;      // C23
/*fragment*/ UDecimal64      : '_Decimal64' ;      // C23
/*fragment*/ Constexpr       : 'constexpr' ;       // C23 (若未来加入，此处预留)

// === 预处理器运算符与特殊关键字 ===
// 这些通常在预处理上下文中识别，但为了完整性可列出

// 预处理器指令关键字（通常由预处理器处理，但词法分析可识别）
/*fragment*/ Define          : 'define' ;          // #define 的一部分
/*fragment*/ IfDef           : 'ifdef' ;           // #ifdef
/*fragment*/ IfNDef          : 'ifndef' ;          // #ifndef
/*fragment*/ Elif            : 'elif' ;
/*fragment*/ EndIf           : 'endif' ;
/*fragment*/ Include         : 'include' ;
/*fragment*/ Line            : 'line' ;
/*fragment*/ Error           : 'error' ;
/*fragment*/ Pragma          : 'pragma' ;
/*fragment*/ Undef           : 'undef' ;
//If              : 'if' ;              // 已在标准关键字中，但预处理器也使用

// 预处理器运算符 (C99 及以后)
/*fragment*/ UPragma         : '_Pragma' ;          // C99

// 扩展关键字 (条件支持)
/*fragment*/ Asm             : 'asm' ;              // 常见扩展
/*fragment*/ Fortra          : 'fortran' ;          // 扩展，罕见


