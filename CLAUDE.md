# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

> 本文件聚焦**技术架构与构建/测试命令**。项目背景、评分标准、小组成员见 `.claude/CLAUDE.md`（项目级指令，已被自动加载，勿与此处重复）。

## 构建与运行

**首选 Docker 构建**（环境一致性，避免本地依赖地狱）：

```bash
docker build -t compiler .                          # 完整构建（含所有依赖安装）
docker build -f Dockerfile.dev -t cphw-dev:latest . # 增量构建（基于已缓存基础镜像，迭代时用这个）

# 运行完整流水线（注意：容器内 ENTRYPOINT 已是 ./build/bin/parser）
docker run --rm -v "$(pwd):/data" compiler \
    --input /data/parser/testfiles/input.rs \
    --lexer-output /data/lex.tsv \
    --parser-output /data/parse.txt \
    --semantic-output /data/semantic.txt \
    --ir-output /data/ir.txt
```

**本地构建**（需 flex/cmake/g++/vcpkg/mingw-w64，用 `sudo ./run.sh` 自动安装依赖）。`run.sh` 会同时编译 Linux 原生版本（`parser/build/bin/parser`）和 Windows 交叉编译版本（`parser/build-win/bin/parser.exe`），然后跑一遍完整流水线。

### ⚠️ 关键：修改词法规则后必须手动重新生成 `lex.yy.cpp`

`CMakeLists.txt` 把 `src/lex.yy.cpp` 直接列入源文件列表，但 **CMake 不会自动调用 flex 重新生成它**。一旦改了 `src/lexer.l`，必须手动执行，否则改动不生效：

```bash
cd parser/src && flex -o lex.yy.cpp lexer.l
# 或在 Docker 内：RUN cd parser/src && flex -o lex.yy.cpp lexer.l（Dockerfile 已含此步）
```

## 测试

```bash
# 语义分析 + IR 集成测试（22 项，覆盖规则 0~9 + 18 类错误检测）
cd parser && bash testfiles/semantic_tests.sh                # 默认用 ./build/bin/parser
bash testfiles/semantic_tests.sh ./build/bin/parser          # 显式指定二进制路径（Docker 内常用）
bash testfiles/semantic_tests.sh /some/abs/path/to/parser    # 任意路径

# 大作业1 语法分析测试
cd parser && bash testfiles/test_all.sh
```

**单测机制**：`semantic_tests.sh` 通过 parser 的 **exit code** 判定——语义正确返回 0，有错误返回 1。脚本用 heredoc 在临时目录内逐个生成 `.rs` 用例并断言 `pass`/`fail`。要加测试用例，直接在脚本里追加 `cat > "$TMPDIR/xxx.rs" << 'EOF' ... EOF` + `run_test "name" "$TMPDIR/xxx.rs" "pass|fail"`。

**单文件手动验证**（最快反馈循环）：

```bash
./parser/build/bin/parser --input parser/testfiles/test_full.rs -q   # exit 0 = 语义通过
./parser/build/bin/parser --input parser/testfiles/test_errors.rs    # 应输出 18 类错误并 exit 1
```

## 架构（Big Picture）

### 单驱动四阶段流水线

`parser/src/main.cpp` 是**唯一**的流水线驱动，严格按序调用，各阶段不可乱序：

```
源文件(.rs)
  │  lex()                          [lexer.l → TokenStream]
  ▼
TokenStream (TSV: Type,Category,Value,Pos,Line,Column)
  │  Parser::parse()                [parser.h，递归下降]
  ▼
AST (Node 树)  ← 共享数据结构，语义+IR 都消费它
  │  SemanticAnalyzer::analyze()    [两遍 visitor]
  ▼
(无错误才继续；有错误则 printErrors + exit 1)
  │  IRGenerator::generate()        [四元式 emit]
  ▼
vector<Quadruple>  →  四元式文本
```

**重要**：语义分析失败时 `main.cpp` 直接 `return 1`，**不会生成 IR**。这是设计而非 bug——IR 生成假设 AST 已通过语义检查。

### AST 是跨阶段的隐式契约

`Node`（`parser.h`）极简：`label` + `children` + `isLeaf` + `line`，类似 DOM。`SemanticAnalyzer` 和 `IRGenerator` 都不直接持有结构化字段，而是通过 `findChild(node, "label字符串")` / `findChildren(...)` **按 label 文本在树里查找子节点**。

这意味着：**`parser.h` 里 `parseXXX` 产生的 label 字符串，是语义/IR 阶段的查找键**。改 parser 的某个 label（比如把 `"LetStmt"` 改名）会**静默破坏** `SemanticAnalyzer.cpp` / `IRGenerator.cpp` 里所有对应的 `findChild(...)` 调用，且编译器不会报错。修改 label 时必须全局搜索该字符串，同步更新三处。

### 两遍语义分析（解决函数前向引用）

`SemanticAnalyzer::analyze()` 分两遍遍历函数列表：
1. **`registerFunction`**：先扫描所有函数声明，注册到 `functions` 表（这样才能在函数体里调用"后面才定义"的函数）
2. **`visitFunction`**：再逐个分析函数体

类型系统 `SType`（`SemanticType.h`）的 `BaseType` 枚举值全部带 **`ST_` 前缀**（`ST_I32`/`ST_VOID`/...）。这是**刻意**的——裸 `VOID` 会和 Windows SDK 的 `VOID` 宏在 mingw 交叉编译时冲突。新增类型枚举值务必延续 `ST_` 前缀。

`SymbolTable`（`Symbol.h`）是栈式作用域（`enterScope`/`exitScope`），`Symbol` 记录 `is_mutable`/`is_assigned`/`has_immutable_ref`/`has_mutable_ref` —— 后两个标志位支撑借用规则检查（错误类型 12/13/15）。

### IR 生成器

`IRGenerator`（`IRGenerator.h`）用 `emit(op, arg1, arg2, result)` 追加 `Quadruple`。临时变量 `newTemp()` → `t0,t1,...`，标签 `newLabel()` → `L0,L1,...`。`in_loop` 计数器 + `break_label`/`continue_label` 成员解决 break/continue 的跳转目标。控制流（if/while/for/loop）通过 `LABEL`/`JUMP`/`JZ`/`JNZ` 四元式 + 短路求值实现。

### GUI 仅在 Windows 构建启用

`CMakeLists.txt` 用 `if(BUILD_WINDOWS OR CMAKE_CROSSCOMPILING)` 包裹整个 GUI（imgui + glfw）编译。**Linux 原生构建是纯 CLI**，`main.cpp` 里所有 GUI 代码用 `#ifdef WINDOWS_BUILD` 守卫。在 Linux 上调试 GUI 代码无效——必须走交叉编译路径。

## 工作约定（来自 `.claude/CLAUDE.md`）

- 中间代码采用**四元式**（quadruple）形式
- 语义错误必须在分析过程中诊断并报告，给出有意义的错误信息（含行号）
- 按 `.claude/plan.md` 推进，每完成一项更新 `.claude/status.md`
- 大作业2 评分：报告 30% + 问题分析/方案设计/编程各 20% + 文献 10%
