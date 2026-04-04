# 0 所有的词法分析单元将会以如下形式输出

**词法分析器的输出将会是一个 csv 文件，也可以按照需要输出为一个超级大数组。**

其中，第一列为 类型，第二列为 种别，第三列为 值，对于下面的例子：

```
let mut x = 42;
if x >= 10 { return x; }
```

| type       | category        | value |
| ---------- | --------------- | ----- |
| keyword    | Let             | let   |
| keyword    | Mut             | mut   |
| identifier | Identifier      | x     |
| delimiter  | Assign          | =     |
| constant   | IntegerConstant | 42    |
| delimiter  | Semicolon       | ;     |
| keyword    | If              | if    |
| identifier | Identifier      | x     |
| operator   | GreaterEqual    | >=    |
| constant   | IntegerConstant | 10    |
| delimiter  | LBrace          | {     |
| keyword    | Return          | return|
| identifier | Identifier      | x     |
| delimiter  | Semicolon       | ;     |
| delimiter  | RBrace          | }     |

事实上，Category 用于区分同一类型下的不同种别。

# 1 类别

| 类型        | 说明   |
| ----------- | ------ |
| keyword     | 关键字 |
| operator    | 运算符 |
| delimiter   | 界符   |
| constant    | 常数   |
| identifier  | 标识符 |
| comment     | 注释   |
| declerator  | 类型声明符 |

# 2 种别

## 2.1 keyword

每个关键字单独一种。

```
Let         : 'let' ;
Mut         : 'mut' ;
Fn          : 'fn' ;
If          : 'if' ;
Else        : 'else' ;
While       : 'while' ;
For         : 'for' ;
In          : 'in' ;
Loop        : 'loop' ;
Break       : 'break' ;
Continue    : 'continue' ;
Return      : 'return' ;
```

## 2.2 operator

所有的运算符如下：

```
Arrow        : '->' ;
DotDot       : '..' ;
Dot          : '.' ;
EqualEqual   : '==' ;
NotEqual     : '!=' ;
GreaterEqual : '>=' ;
LessEqual    : '<=' ;
Greater      : '>' ;
Less         : '<' ;
Plus         : '+' ;
Minus        : '-' ;
Star         : '*' ;
Slash        : '/' ;
Ampersand    : '&' ;
```

## 2.3 delimiter

```
Assign       : '=' ;
Semicolon    : ';' ;
Colon        : ':' ;
Comma        : ',' ;
LParen       : '(' ;
RParen       : ')' ;
LBrace       : '{' ;
RBrace       : '}' ;
LBracket     : '[' ;
RBracket     : ']' ;
End          : '#' ;
```

## 2.4 constant

常量只考虑了单个的语法单元，例如 `42`，`"123"`，`3.14`，`'a'`。`1+1` 在词法分析器当中不是常量。常量有如下四种：

```
IntegerConstant      : 整数常量，如 42, 0xFF, 0b1010, 1_000_000 等
FloatingConstant     : 浮点数常量，如 3.14, 2.5e10, 1.0f 等
StringConst          : 字符串常量，如 "hello", "world\n" 等
CharacterConstant    : 字符常量，如 'a', '\n', '\x41' 等
```

需要注意的是，如果 StringConst 当中出现了换行符，对于程序的输出有很大的影响，因此提供了转义序列。采取如下方式的原因是，如果原字符串当中有 `\r` 的话，程序输入字符串当中的换行符，和程序当中的字符串将会无法区分。例如如下程序输入字符串：

```c
"123\r\\
    ^^  ^
    1   2
123"
```

为了区分 `1` 和 `2` 处的换行符，特别做了下面的转义。

| 转义前（C 语言方式表示） | 转义后 |
| ----------------------- | ------ |
| $                       | $$     |
| (空格)                  | $_     |
| \r                      | $r     |
| \n                      | $n     |
| \t                      | $t     |
| \0                      | $0     |

## 2.5 identifier

只有一种：

```
Identifier   : 标识符，以字母或下划线开头，后跟字母、数字或下划线
```

## 2.6 comment

```
LineComment    : 行注释，以 // 开头
BlockComment   : 块注释，以 /* 开头，以 */ 结尾
```

注释的输出会经过 `rebuild_string` 函数处理，将特殊字符转义。

## 2.7 declerator

类型声明符用于声明变量或函数的类型：

```
I32          : 'i32' ;
```

# 3 输出格式示例

词法分析器的输出首行为表头：

```
TYPE 	CATEGORY 	VALUE 
```

之后每行一个词法单元，格式为 `类型\t种别\t值`，例如：

```
keyword	Let	let
keyword	Mut	mut
identifier	Identifier	x
delimiter	Assign	=
constant	IntegerConstant	42
delimiter	Semicolon	;
comment	LineComment	//$_$n
```

# 参考文献

[antlr/grammars-v4: Grammars written for ANTLR v4; expectation that the grammars are free of actions.](https://github.com/antlr/grammars-v4)

[C语言运算符优先级和结合性一览表（非常详细） - C 语言中文网](https://c.biancheng.net/view/oblaq24.html)

[C language - cppreference.com](https://en.cppreference.com/w/c/language.html)

[自己动手写编译器 — 自己动手写编译器](https://pandolia.net/tinyc/index.html)

[北京大学编译实践课程在线文档](https://pku-minic.github.io/online-doc/)
