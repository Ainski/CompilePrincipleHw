# 0 所有的词法分析单元将会以如下形式输出

**词法分析器的输出将会是一个csv文件，也可以按照需要输出为一个超级大数组。**

其中，第一列为 类型，第二列为 种别，第三列为 值，对于下面c语言的例子：

```c++
while (i >=j ) i-- ;
int a= 0;
int*(int,int) p
```

| type       | category   | value |
| ---------- | ---------- | ----- |
| keyword    | While      | while |
| operator   | Lparen     | (     |
| identifier | Identifier | i     |
| operator   | Ge         | >=    |
| identifier | Identifier | j     |
| operator   | Rparen     | )     |
| identifier | Identifier | i     |
| operator   | Dec        | --    |
| delimiter  | Delimiter  | ;     |

事实上，Category仅仅在运算符号时与上一级有区别。

# 1 类别

| 类型        | 说明   |
| ----------- | ------ |
| keyword     | 关键字 |
| operator    | 运算符 |
| delimiter   | 界符   |
| constant    | 常数   |
| identifier  | 标识符 |
| comment     | 注释   |
| declaration | 声明   |

# 2 种别

## 2.1 keyword

每个关键字单独一种，所有的类型关键字 专门定义一个类别 因为还有int*等等的情况

```c
// C89/C90 标准关键字
//Auto            : 'auto' ;
Break           : 'break' ;
Case            : 'case' ;
//Char            : 'char' ;
//Const           : 'const' ;
Continue        : 'continue' ;
Default         : 'default' ;
Do              : 'do' ;
//Double          : 'double' ;
Else            : 'else' ;
//Enum            : 'enum' ;
//Extern          : 'extern' ;
//Float           : 'float' ;
For             : 'for' ;
Goto            : 'goto' ;
If              : 'if' ;
//Int             : 'int' ;
//Long            : 'long' ;
//Register        : 'register' ;
Return          : 'return' ;
//Short           : 'short' ;
//Signed          : 'signed' ;

//Static          : 'static' ;
//Struct          : 'struct' ;
Switch          : 'switch' ;
//Typedef         : 'typedef' ;
//Union           : 'union' ;
//Unsigned        : 'unsigned' ;
//Void            : 'void' ;
//Volatile        : 'volatile' ;
While           : 'while' ;

// C99 新增关键字
//Inline          : 'inline' ;
//Restrict        : 'restrict' ;


// C11 新增关键字
UAlignas        : '_Alignas' ;        // C11
UGeneric        : '_Generic' ;        // C11
UStaticAssert   : '_Static_assert' ;  // C11
UBVA 			: '__builtin_va_arg'
UBO				: '__builtin_offsetof'
// 扩展关键字 (条件支持)
Asm             : 'asm' ;              // 常见扩展
Fortran          : 'fortran' ;          // 扩展，罕见
UAsm : '__asm'
UAsmU : '__asm__'
```

## 2.2 operator

所有的操作符请参考[C语言运算符优先级和结合性一览表（非常详细） - C语言中文网](https://c.biancheng.net/view/oblaq24.html)

以下是每一个种别**约定**的说明

```c
Arrow            : '->' ;
Inc              : '++' ;
Dec              : '--' ;
LShiftAssign     : '<<=' ;
RShiftAssign     : '>>=' ;
PlusAssign       : '+=' ;
MinusAssign      : '-=' ;
StarAssign       : '*=' ;
SlashAssign      : '/=' ;
PercentAssign    : '%=' ;
AmpAssign        : '&=' ;
BitorAssign      : '|=' ;
CaretAssign      : '^=' ;
Eq               : '==' ;
Ne               : '!=' ;
Le               : '<=' ;
Ge               : '>=' ;
LShift           : '<<' ;
RShift           : '>>' ;
And              : '&&' ;
Or               : '||' ;

LParen           : '(' ;
RParen           : ')' ;
LBrack           : '[' ;
RBrack           : ']' ;
LBrace           : '{' ;
RBrace           : '}' ;
Comma            : ',' ;
Dot              : '.' ;
Pos              : '+' ;   // 正号/加号
Neg              : '-' ;   // 负号/减号
Star             : '*' ;   // 乘号/解引用
Slash            : '/' ;   // 除号
Percent          : '%' ;   // 取模
Amp              : '&' ;   // 按位与/取地址
Bitor            : '|' ;   // 按位或
Caret            : '^' ;   // 按位异或
Tilde            : '~' ;   // 按位取反
Exclam           : '!' ;   // 逻辑非
Question         : '?' ;   // 条件运算符的问号
Colon            : ':' ;   // 条件运算符的冒号
Assign           : '=' ;   // 赋值
Lt               : '<' ;   // 小于
Gt               : '>' ;   // 大于
Underscore       : '_' ;   // 下划线
Hash             : '#' ;   // 预处理指令的井号
SingleQuote      : '\'' ;  // 单引号字符
DoubleQuote      : '"' ;  // 双引号字符

Sizeof          : 'sizeof' ;
UAlignof        : '_Alignof'
```

## 2.3 delimiter

只有一种

```
Delimiter ;
```

## 2.4 constant

常量只考虑了单个的语法单元，例如1，"123"，4.5 。`1+1` 在词法分析器当中不是常量。 因此，常量只有如下四种。

```
StringConst
CharConst
IntConst
FloatConst
```

需要注意的是，如果StringConst 当中出现了换行符，对于程序的输出有很大的影响，因此我提供了转移序列，转义序列如下。采取如下方式的原因是，如果原字符串当中有`\r` 的话，程序输入字符串当中的换行符，和程序当中的字符串将会无法区分。例如如下程序输入字符串

```c
"123\r\\
    ^^  ^
    1   2
123"
```

为了区分`1` 和 `2`处的换行符，特别做了下面的转义。

| 转义前（C语言方式表示） | 转义后 |
| ----------------------- | ------ |
| $                       | $$     |
| (空格)                  | $_     |
| \r                      | $r     |
| \n                      | $n     |
| \t                      | $t     |
| \0                      | $0     |
| 不可见字符              | $.     |



## 2.5 identifier

只有一种

## 2.6 comment

```
LineComment
BlockComment
```

## 2.7 declarator

```
/************************** 1. 类型修饰符（符号/长度限定）**************************/
Signed      : signed;       // 有符号类型
Unsigned    : unsigned;     // 无符号类型

/************************** 2. 存储类说明符（生命周期/作用域/存储位置）**************************/
Auto        : auto;         // 自动存储期（局部变量默认）
Static      : static;       // 静态存储期/内部链接
Extern      : extern;       // 外部链接
Register    : register;     // 寄存器存储建议
UThreadULocal: _Thread_local;// 线程局部存储（C11标准）
Typedef     : typedef;      // 类型别名定义

/************************** 3. 基础数据类型关键字 **************************/
Void        : void;         // 无类型
Char        : char;         // 字符类型
Short       : short;        // 短整型
Int         : int;          // 整型
Long        : long;         // 长整型
Float       : float;        // 单精度浮点型
Double      : double;       // 双精度浮点型
UBool       : _Bool;        // C99布尔类型
UComplex    : _Complex;     // C99复数类型
UImaginary  : _Imaginary;   // C99虚数类型
UM128       : __m128;       // 编译器扩展：SIMD单精度浮点
UM128d      : __m128d;      // 编译器扩展：SIMD双精度浮点
UM128i      : __m128i;      // 编译器扩展：SIMD整数

/************************** 4. 类型构造符（自定义复合类型）**************************/
Struct      : struct;       // 结构体
Union       : union;        // 共用体
Enum        : enum;         // 枚举

/************************** 5. 类型限定符（内存/访问特性）**************************/
Const       : const;        // 只读限定
Restrict    : restrict;     // 指针唯一访问限定（C99）
Volatile    : volatile;     // 易变限定（禁止编译器优化）
UVolatile   : __volatile__; // 编译器扩展版volatile
UAtomic     : _Atomic;      // C11原子类型限定

/************************** 6. 函数/代码特性修饰符 **************************/
Inline      : inline;       // 内联函数（C99）
UInline     : __inline__;   // 编译器扩展版inline
UNoreturn   : _Noreturn;    // C11无返回值函数
UStdcall    : __stdcall;    // 调用约定：标准调用
UCdecl      : __cdecl;      // 调用约定：C调用
UClrcall    : __clrcall;    // 调用约定：CLR调用
UFastcall   : __fastcall;   // 调用约定：快速调用
UThiscall   : __thiscall;   // 调用约定：this指针调用
UVectorcall : __vectorcall; // 调用约定：向量调用

/************************** 7. 编译器扩展关键字（通用）**************************/
UTypeof     : __typeof__;   // 编译器扩展：获取类型信息
UExtension  : __extension__;// 编译器扩展：启用扩展特性
UDeclspec   : __declspec;   // 微软编译器扩展（如dllimport）
UAttribute  : __attribute__;// GCC编译器扩展（如noreturn/packed）
```



# 参考文献

[antlr/grammars-v4: Grammars written for ANTLR v4; expectation that the grammars are free of actions.](https://github.com/antlr/grammars-v4)

[C语言运算符优先级和结合性一览表（非常详细） - C语言中文网](https://c.biancheng.net/view/oblaq24.html)

[C language - cppreference.com](https://en.cppreference.com/w/c/language.html)

请一定阅读一个已经写好的clexer [CompilePrincipleHw/antlr4/src/main/clexer/OverC.g4 at chr_feat · Ainski/CompilePrincipleHw](https://github.com/Ainski/CompilePrincipleHw/blob/chr_feat/antlr4/src/main/clexer/OverC.g4)

[自己动手写编译器 — 自己动手写编译器](https://pandolia.net/tinyc/index.html)

[北京大学编译实践课程在线文档](https://pku-minic.github.io/online-doc/)
