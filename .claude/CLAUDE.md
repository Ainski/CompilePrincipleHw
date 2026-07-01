# CompilePrincipleHw 大作业2 — 中间代码生成器

## 项目概述

同济大学编译原理课程大作业2：在已有的词法分析器 + 递归下降语法分析器基础上，增加**语义分析**和**中间代码生成**功能。

小组成员：程浩然、黄艺鑫、韦畅

## 前一阶段（大作业1）工作成果

大作业1 已完成：
- **词法分析器**：基于 Flex，识别关键字、标识符、运算符、界符、常数、注释、类型声明符共 7 大类词法单元
- **递归下降语法分析器**：覆盖基础程序、变量声明、表达式、选择结构、循环结构、引用与借用、表达式块、数组、元组等语法规则
- **CLI 模式**：支持 `--input`、`--lexer-output`、`--parser-output`、`--print-tokens`、`-q/--quiet` 等参数
- **GUI 模式**：基于 Dear ImGui + GLFW
- **测试用例**：12 组测试用例，覆盖全部语法规则

## 可执行文件用法

```bash
# Docker 构建（必须用 Docker）
docker build -t compiler .
docker run --rm -v $(pwd):/data compiler --input /data/source.rs --lexer-output /data/lex.tsv --parser-output /data/parse.txt

# 本地运行（需要 flex, cmake, g++, vcpkg 等依赖）
sudo ./run.sh                                    # 一键构建并运行
./parser --input <源文件> --lexer-output <词法输出.tsv> --parser-output <语法输出.txt>
./parser --input <源文件> --print-tokens         # 打印词法单元到控制台
./parser --input <源文件> --gui                   # 启动 GUI 界面
./parser --input <源文件> -q                      # 静默模式
```

## 构建方式

**必须使用 Docker 构建**。Dockerfile 位于项目根目录，基于 Ubuntu 22.04，自动安装所有依赖（flex, cmake, mingw-w64, vcpkg, glfw3 等）。

```bash
docker build -t compiler .
```

## 工作约定

- **按照 plan.md 中的计划逐步推进**，每完成一项在 status.md 中更新状态
- 可以利用前一阶段的词法分析器和语法分析器成果，在其基础上扩展
- 中间代码建议采用**四元式**（quadruple）形式
- 语义错误需在分析过程中诊断并报告，给出有意义的错误信息

## 大作业2 评分标准

| 内容 | 占比 | 要求 |
|------|------|------|
| 问题分析能力 | 20% | 说明语义分析和中间代码生成原理 |
| 系统方案设计能力 | 20% | 报告体现总体设计和详细设计 |
| 编程能力 | 20% | 独立编程实现全部功能 |
| 撰写报告能力 | 30% | 表达通顺、结构清晰、内容完整 |
| 查阅文献资料能力 | 10% | 列出所查阅的文献资料 |

## 报告要求

- 设计文档 1 份
- 程序源代码、可执行代码 1 份
- 程序实例与结果截屏
- 报告 PPT 1 份

---

## 作业3（课程设计）里程碑路线图

> 课程设计（独立计分）：在前两阶段基础上新增目标代码生成。截止 2026-07-31，答辩 2026-08-03。
> 前阶段 `plan.md`/`status.md` 为大作业2 归档；作业3 用 `milestones/` 体系。
> 架构决策：**A2**（保留 lex→parse→AST，semantic+IR 合并单 pass，报告论证逻辑一遍）+ **B1**（x86-64 AT&T 汇编，栈式映射优先）。

### M1: 架构合并 ✅（2026-07-02）
目标：SemanticAnalyzer 嫁接 IR 生成，单次遍历完成语义检查 + 四元式 emit；IRGenerator 已删除。
详细：`milestones/M1.md`

### M2: 规则 7.x 补齐 ✅（2026-07-02）
目标：if/loop/block 统一表达式化，支持块值/选择/循环表达式（7.0-7.4）。
详细：`milestones/M2.md`

### M3: 目标代码生成（x86-64 汇编）✅（2026-07-02）
目标：四元式 → 可汇编执行 `.s`；fibonacci(10)=55 端到端验证通过。
详细：`milestones/M3.md`

### M4: 报告、测试加固与交付（当前）
目标：`report3`（含 AI 章节 + 一遍论证）+ 答辩材料 + v3.0.0。
前置：M3　详细：`milestones/M4.md`

### 跨里程碑约束
- 现有 22 项 `semantic_tests` 不可回归（每次改动后跑）
- 新代码附测试（构建验证 + 手动验证最低）
- 新增枚举值延续 `ST_` 前缀（规避 Windows 宏冲突）
- 目标代码先正确后优化（栈式映射优先于寄存器分配）
- 不破坏 lex→parse→AST 主干（A2 原则）
