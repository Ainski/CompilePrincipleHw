# CompilePrincipleHw

同济大学编译原理课程大作业：类 Rust 语言的词法分析器与递归下降语法分析器。

小组成员：程浩然、黄艺鑫、韦畅

## 功能特性

- **词法分析**：基于 Flex 实现，识别关键字、标识符、运算符、界符、常数、注释、类型声明符共 7 大类词法单元
- **语法分析**：基于递归下降解析器实现，覆盖基础程序、变量声明、表达式、选择结构、循环结构、引用与借用、表达式块、数组、元组等语法规则
- **CLI 模式**：命令行接口，支持指定输入文件及输出路径
- **GUI 模式**：基于 Dear ImGui + GLFW 的图形界面，可视化展示词法与语法分析结果
- **静默模式**：`-q` / `--quiet` 标志，抑制 ASCII Banner 输出
- **批量测试**：内置 12 组测试用例，一键运行并输出测试报告
- **交叉编译**：在 Linux 环境下同时构建 Linux 原生版本与 Windows `.exe` 版本

## 快速开始

```bash
sudo ./run.sh            # 安装依赖 → flex 生成代码 → CMake 编译（Linux + Windows） → 运行分析
sudo ./run.sh --clean    # 清理所有生成文件
sudo ./run.sh --help     # 查看使用说明
```

脚本会自动检查并安装所需依赖，首次运行可能需要较长时间。

### 命令行参数

```bash
./parser --input <源文件> --lexer-output <词法输出.tsv> --parser-output <语法输出.txt>
```

| 参数 | 说明 |
|------|------|
| `--input` | 输入文件路径（类 Rust 源文件） |
| `--lexer-output` | 词法分析输出文件路径（TSV 格式） |
| `--parser-output` | 语法分析输出文件路径 |
| `--print-tokens` | 将词法单元打印到控制台 |
| `--gui` | 启动 GUI 界面 |
| `-q, --quiet` | 静默模式，不输出 ASCII Banner |

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

项目包含 12 组测试用例，覆盖课程要求的全部语法规则：

```bash
cd parser && bash testfiles/test_all.sh
```

测试用例与覆盖的语法规则对应关系：

| 测试文件 | 覆盖规则 |
|----------|----------|
| `test_1_1` ~ `test_1_5` | 基础程序结构、语句、返回语句、函数输入输出 |
| `test_2` | 变量声明与赋值 |
| `test_3` | 基本表达式、比较/加减/乘除运算、函数调用 |
| `test_4` | if 选择结构 |
| `test_5` | while / for / loop 循环、break / continue |
| `test_6` | 引用与借用 |
| `test_8` | 数组类型、数组表达式、数组元素 |
| `test_9` | 元组类型、元组表达式、元组元素 |

## 项目结构

```
.
├── run.sh                  # 一键构建与运行脚本
├── readme.md
├── docs/
│   ├── assignment.md       # 大作业需求说明（词法规则、语法规则、评分标准）
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
    │   ├── Type.h          # 类型系统
    │   └── gui.h           # GUI 接口
    ├── src/
    │   ├── lexer.l         # Flex 词法规则源文件
    │   ├── main.cpp        # 程序入口
    │   ├── TokenStream.cpp
    │   ├── Type.cpp
    │   └── gui.cpp
    └── testfiles/          # 测试用例及预期输出
```

## 文档

- [词法分析器输出规范](docs/promised_lex_output.md) — 词法单元的类型、种别、值的完整定义
- [大作业需求说明](docs/assignment.md) — 词法规则、语法规则、评分标准
- [参考资料](docs/References.md) — 开发过程中参考的文献与资源
- [设计报告](docs/report/report.pdf) — 完整的设计与说明文档
