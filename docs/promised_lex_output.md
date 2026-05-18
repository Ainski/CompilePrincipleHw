# 词法分析器输出规范

> 本文档定义了类 Rust 词法分析器的输出格式，包括词法单元的三大属性（类型、种别、值）、各类别与种别的完整枚举，以及转义规则和格式示例。

---

## 1 输出格式总览

词法分析器的输出为一个 TSV（制表符分隔）文件，每行一个词法单元，包含三列：

| 列 | 含义 | 说明 |
|----|------|------|
| type | 类型 | 词法单元所属的大类 |
| category | 种别 | 同一类型下的具体分类 |
| value | 值 | 词法单元的原始文本 |

### 示例

输入：

```rust
let mut x = 42;
if x >= 10 { return x; }
```

输出：

| type | category | value |
|------|----------|-------|
| keyword | Let | let |
| keyword | Mut | mut |
| identifier | Identifier | x |
| delimiter | Assign | = |
| constant | IntegerConstant | 42 |
| delimiter | Semicolon | ; |
| keyword | If | if |
| identifier | Identifier | x |
| operator | GreaterEqual | >= |
| constant | IntegerConstant | 10 |
| delimiter | LBrace | { |
| keyword | Return | return |
| identifier | Identifier | x |
| delimiter | Semicolon | ; |
| delimiter | RBrace | } |

---

## 2 类型（type）列表

| 类型 | 说明 |
|------|------|
| keyword | 关键字 |
| operator | 运算符 |
| delimiter | 界符 |
| constant | 常数 |
| identifier | 标识符 |
| comment | 注释 |
| declerator | 类型声明符 |

---

## 3 种别（category）列表

### 3.1 keyword

每个关键字单独一种：

| 种别 | 原文 |
|------|------|
| Let | `let` |
| Mut | `mut` |
| Fn | `fn` |
| If | `if` |
| Else | `else` |
| While | `while` |
| For | `for` |
| In | `in` |
| Loop | `loop` |
| Break | `break` |
| Continue | `continue` |
| Return | `return` |

### 3.2 operator

| 种别 | 符号 |
|------|------|
| Arrow | `->` |
| DotDot | `..` |
| Dot | `.` |
| EqualEqual | `==` |
| NotEqual | `!=` |
| GreaterEqual | `>=` |
| LessEqual | `<=` |
| Greater | `>` |
| Less | `<` |
| Plus | `+` |
| Minus | `-` |
| Star | `*` |
| Slash | `/` |
| Ampersand | `&` |

### 3.3 delimiter

| 种别 | 符号 |
|------|------|
| Assign | `=` |
| Semicolon | `;` |
| Colon | `:` |
| Comma | `,` |
| LParen | `(` |
| RParen | `)` |
| LBrace | `{` |
| RBrace | `}` |
| LBracket | `[` |
| RBracket | `]` |
| End | `#` |

### 3.4 constant

常量只考虑单个语法单元（如 `42`、`"123"`、`3.14`、`'a'`）。`1+1` 在词法分析器中不是常量。

| 种别 | 说明 | 示例 |
|------|------|------|
| IntegerConstant | 整数常量 | `42`、`0xFF`、`0b1010`、`1_000_000` |
| FloatingConstant | 浮点数常量 | `3.14`、`2.5e10`、`1.0f` |
| StringConst | 字符串常量 | `"hello"`、`"world\n"` |
| CharacterConstant | 字符常量 | `'a'`、`'\n'`、`'\x41'` |

#### 字符串转义规则

当字符串中出现特殊字符时，需进行转义以避免与程序本身的换行符混淆。例如，以下输入字符串中位置 `1` 处的 `\r` 和位置 `2` 处的实际换行需要区分：

```
"123\r\n
    ^^  ^
    1   2
123"
```

转义规则如下：

| 原始字符（C 语言表示） | 转义后 |
|----------------------|--------|
| `$` | `$$` |
| （空格） | `$_` |
| `\r` | `$r` |
| `\n` | `$n` |
| `\t` | `$t` |
| `\0` | `$0` |

### 3.5 identifier

| 种别 | 说明 |
|------|------|
| Identifier | 以字母或下划线开头，后跟字母、数字或下划线 |

### 3.6 comment

| 种别 | 说明 |
|------|------|
| LineComment | 行注释，以 `//` 开头 |
| BlockComment | 块注释，以 `/*` 开头，`*/` 结尾 |

注释的输出会经过 `rebuild_string` 函数处理，将特殊字符转义。

### 3.7 declerator

| 种别 | 原文 |
|------|------|
| I32 | `i32` |

---

## 4 输出格式示例

词法分析器输出首行为表头：

```
TYPE 	CATEGORY 	VALUE
```

之后每行一个词法单元，字段之间以制表符 `\t` 分隔：

```
keyword	Let	let
keyword	Mut	mut
identifier	Identifier	x
delimiter	Assign	=
constant	IntegerConstant	42
delimiter	Semicolon	;
comment	LineComment	//$_$n
```
