# CompilePrincipleHw

同济大学编译原理课程大作业：类 Rust 语言的词法分析器、递归下降语法分析器、语义分析器与中间代码生成器。

小组成员：程浩然、黄艺鑫、韦畅

## 功能特性

- **词法分析**：基于 Flex 实现，识别关键字、标识符、运算符、界符、常数、注释、类型声明符共 7 大类词法单元
- **语法分析**：基于递归下降解析器实现，覆盖基础程序、变量声明、表达式、选择结构、循环结构、引用与借用、表达式块、数组、元组等语法规则
- **语义分析**：基于 visitor 模式遍历语法树，实现类型检查、作用域分析、借用追踪等 18 类语义错误检测
- **中间代码生成**：生成四元式（quadruple）中间代码，覆盖算术运算、比较运算、控制流、函数调用等
- **CLI 模式**：命令行接口，支持指定输入文件及各阶段输出路径
- **GUI 模式**：基于 Dear ImGui + GLFW 的图形界面，可视化展示词法与语法分析结果
- **静默模式**：`-q` / `--quiet` 标志，抑制 ASCII Banner 输出
- **交叉编译**：在 Linux 环境下同时构建 Linux 原生版本与 Windows `.exe` 版本

### 语义分析覆盖规则

**必做规则**：
- 0.1~0.3 基础属性（mut、i32、左值）
- 1.1~1.5 程序结构（基础程序、语句、返回、函数输入输出）
- 2.0~2.2 变量声明与赋值（含 shadowing、类型推断）
- 3.1~3.5 表达式（基本表达式、比较/加减/乘除运算、函数调用）
- 4.1 选择结构（if）
- 5.0~5.1 循环结构（while）

**选做规则**：
- 4.2~4.3 else / else if
- 5.2~5.4 for / loop / break / continue
- 6.1~6.4 不可变变量、不可变引用、可变引用、借用（解引用）
- 8.1~8.3 数组类型、数组表达式、数组元素访问
- 9.1~9.3 元组类型、元组表达式、元组元素访问

### 语义错误检测（18 类）

| 编号 | 错误类型 | 示例 |
|------|----------|------|
| 1 | 返回类型不一致 | 函数声明 `-> i32` 但 `return ;` |
| 2 | 赋值给未声明变量 | `a=32;`（a 未声明） |
| 3 | 赋值类型不一致 | `let mut a:i32; a=1==1;` |
| 4 | 右值未提前声明 | `let mut b:i32=a;`（a 未声明） |
| 5 | 右值未提前赋值 | `let mut a:i32; let mut b:i32=a;` |
| 6 | 不可变变量二次赋值 | `let c:i32=1; c=2;` |
| 7 | 实参数量不一致 | 调用参数数量与定义不匹配 |
| 8 | 无返回值函数作为右值 | `let mut a=void_func();` |
| 9 | 实参类型不一致 | 传参类型与形参不匹配 |
| 10 | break 不在循环体内 | 函数中直接 `break;` |
| 11 | continue 不在循环体内 | 函数中直接 `continue;` |
| 12 | 可变引用与其他引用共存 | `&a` 和 `&mut a` 同时存在 |
| 13 | 从不可变变量创建可变引用 | `let a:i32=1; let mut b=&mut a;` |
| 14 | 对非引用类型解引用 | `let mut b=*a;`（a 非 ref） |
| 15 | 不可变引用修改指向数据 | `let b=&a; *b=2;` |
| 16 | 数组初始化元素数量不一致 | `a=[1,2,3];`（a 声明为 [i32;2]） |
| 17 | 元组初始化元素数量不一致 | `a=(1,2,3);`（a 声明为 (i32,i32)） |

## 快速开始

```bash
sudo ./run.sh            # 安装依赖 → flex → CMake 编译（Linux + Windows） → 运行完整流水线
sudo ./run.sh --clean    # 清理所有生成文件
sudo ./run.sh --help     # 查看使用说明
```

脚本会自动检查并安装所需依赖，首次运行可能需要较长时间。

### 命令行参数

```bash
./parser --input <源文件> \
    --lexer-output <词法输出.tsv> \
    --parser-output <语法输出.txt> \
    --semantic-output <语义分析输出.txt> \
    --ir-output <中间代码输出.txt>
```

| 参数 | 说明 |
|------|------|
| `--input` | 输入文件路径（类 Rust 源文件） |
| `--lexer-output` | 词法分析输出文件路径（TSV 格式） |
| `--parser-output` | 语法分析输出文件路径 |
| `--semantic-output` | 语义分析输出文件路径（错误信息） |
| `--ir-output` | 中间代码输出文件路径（四元式序列） |
| `--print-tokens` | 将词法单元打印到控制台 |
| `--gui` | 启动 GUI 界面 |
| `-q, --quiet` | 静默模式，不输出 ASCII Banner |

### 中间代码格式示例

```
(FUNC_BEGIN, , , fibonacci)
(PARAM, , , mut n)
(ASSIGN, 0, , _t1)
(LE, n, _t1, _t2)
(JZ, _t2, , _L2)
(RETURN, n, , )
(LABEL, , , _L2)
(ASSIGN, 0, , _t3)
(ASSIGN, _t3, , a)
...
(FUNC_END, , , fibonacci)
```

### Docker 构建

```bash
# 完整构建
docker build -t compiler .

# 增量构建（基于已缓存的镜像，仅重新编译代码）
docker build -f Dockerfile.dev -t compile-principle-hw-dev:latest .

# 运行
docker run --rm -v $(pwd):/data compiler \
    --input /data/parser/testfiles/input.rs \
    --lexer-output /data/lex.tsv \
    --parser-output /data/parse.txt \
    --semantic-output /data/semantic.txt \
    --ir-output /data/ir.txt
```

## 依赖

以下依赖由 `run.sh` 自动检测并安装：

| 依赖 | 用途 |
|------|------|
| flex 2.6.4 | 词法分析器生成 |
| g++ / build-essential | C++ 编译 |
| cmake | 构建系统 |
| make | 构建工具 |
| vcpkg | C++ 包管理 |
| glfw3 (x64-linux) | GUI 窗口系统（Linux） |
| glfw3 (x64-mingw-dynamic) | GUI 窗口系统（Windows 交叉编译） |
| mingw-w64 | Windows 交叉编译工具链 |

## 测试

### 语法分析测试（大作业1）

```bash
cd parser && bash testfiles/test_all.sh
```

### 语义分析与中间代码测试（大作业2）

```bash
cd parser && bash testfiles/semantic_tests.sh
```

22 项测试全部通过，包含：
- 12 组原有语法分析测试
- 综合正确用例（`test_full.rs`：30+ 函数，覆盖全部规则 0~9）
- 17 个语义错误检测用例（`test_errors.rs`：检测 18 个语义错误）

测试用例与覆盖的语法规则对应关系：

| 测试文件 | 覆盖规则 |
|----------|----------|
| `test_1_1` ~ `test_1_5` | 基础程序结构、语句、返回语句、函数输入输出 |
| `test_2` | 变量声明与赋值 |
| `test_3` | 基本表达式、比较/加减/乘除运算、函数调用 |
| `test_4` | if / else / else if 选择结构 |
| `test_5` | while / for / loop 循环、break / continue |
| `test_6` | 不可变变量、不可变引用、可变引用、借用 |
| `test_8` | 数组类型、数组表达式、数组元素 |
| `test_9` | 元组类型、元组表达式、元组元素 |
| `test_full` | 全规则综合测试（含 fibonacci 完整程序） |
| `test_errors` | 18 类语义错误检测 |

## 项目结构

```
.
├── run.sh                  # 一键构建与运行脚本
├── readme.md
├── Dockerfile              # 完整 Docker 构建（含依赖安装）
├── Dockerfile.dev          # 增量构建（基于已缓存镜像）
├── docs/
│   ├── assignment.md       # 大作业1 需求说明
│   ├── assignment2.md      # 大作业2 需求说明（语义分析+中间代码）
│   ├── promised_lex_output.md  # 词法分析器输出格式规范
│   ├── References.md       # 参考资料列表
│   └── report/
│       ├── report.tex      # 报告 LaTeX 源文件
│       ├── report.pdf      # 报告 PDF
│       └── figures/        # 报告插图
└── parser/
    ├── CMakeLists.txt      # CMake 构建配置
    ├── include/            # 头文件
    │   ├── CLI/            # CLI11 命令行解析库
    │   ├── config.h
    │   ├── lexer.h         # 词法分析器接口
    │   ├── parser.h        # 语法分析器接口
    │   ├── Token.h         # 词法单元定义
    │   ├── TokenStream.h   # 词法单元流
    │   ├── Type.h          # 类型系统（语法分析阶段）
    │   ├── SemanticType.h  # 语义类型系统（i32/bool/void/ref/array/tuple）
    │   ├── Symbol.h        # 符号表（Symbol + SymbolTable）
    │   ├── SemanticAnalyzer.h  # 语义分析器
    │   ├── IRGenerator.h   # 中间代码生成器
    │   └── gui.h           # GUI 接口
    ├── src/
    │   ├── lexer.l         # Flex 词法规则源文件
    │   ├── main.cpp        # 程序入口
    │   ├── TokenStream.cpp
    │   ├── Type.cpp
    │   ├── SemanticType.cpp    # 语义类型 toString / equals
    │   ├── Symbol.cpp          # 符号表作用域管理
    │   ├── SemanticAnalyzer.cpp # 语义分析器实现（~700行）
    │   ├── IRGenerator.cpp     # 四元式生成器实现（~600行）
    │   └── gui.cpp
    └── testfiles/          # 测试用例及测试脚本
        ├── test_all.sh          # 语法分析批量测试
        ├── semantic_tests.sh    # 语义分析批量测试
        ├── test_1_1.rs ~ test_9.rs  # 语法分析测试用例
        ├── test_full.rs         # 语义分析综合正确用例
        └── test_errors.rs      # 语义分析错误检测用例
```

## 文档

- [大作业1 需求说明](docs/assignment.md) — 词法规则、语法规则、评分标准
- [大作业2 需求说明](docs/assignment2.md) — 语义分析规则、中间代码生成要求
- [词法分析器输出规范](docs/promised_lex_output.md) — 词法单元的类型、种别、值的完整定义
- [参考资料](docs/References.md) — 开发过程中参考的文献与资源
- [设计报告](docs/report/report.pdf) — 完整的设计与说明文档
