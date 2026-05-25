# 大作业2 进度状态

最后更新：2026-05-26

## 当前进度概览

| 阶段 | 状态 | 说明 |
|------|------|------|
| 0. 项目基础设施 | ✅ 完成 | Dockerfile + Dockerfile.dev + CMakeLists.txt |
| 1. 符号表与类型系统 | ✅ 完成 | Symbol, SymbolTable, SemanticType, FunctionInfo |
| 2. 语义分析器框架 | ✅ 完成 | SemanticAnalyzer visitor 模式 |
| 3. 必做语义规则 | ✅ 完成 | 规则 0.1~5.4 全部语义检查 |
| 4. 中间代码生成 | ✅ 完成 | 四元式 IR 生成（算术/比较/控制流/函数调用） |
| 5. 集成测试 | ✅ 完成 | 22/22 通过（正确+错误用例） |
| 6. 选做规则 | ✅ 完成 | else/for/loop/引用/借用/数组/元组 |
| 7. 文档 | 📋 未开始 | 设计报告 + PPT |

## 详细状态

### 阶段 0: 项目基础设施
- [x] 0.1 分阶段 Dockerfile（依赖缓存层）+ Dockerfile.dev 增量构建
- [x] 0.2 新增语义分析模块文件结构
- [x] 0.3 更新 CMakeLists.txt

### 阶段 1: 符号表与类型系统基础
- [x] 1.1 Symbol 类（含借用追踪 has_immutable_ref / has_mutable_ref）
- [x] 1.2 SymbolTable 类（嵌套作用域）
- [x] 1.3 SemanticType 类型系统（ST_ 前缀避免宏冲突）
- [x] 1.4 FunctionInfo 类

### 阶段 2: 语义分析器框架
- [x] 2.1 SemanticAnalyzer 类
- [x] 2.2 错误报告机制（Error: <描述> at line <行号>）
- [x] 2.3 节点遍历分发

### 阶段 3: 必做语义规则实现
- [x] 3.1 规则 0.1 mut
- [x] 3.2 规则 0.2 i32
- [x] 3.3 规则 0.3 左值
- [x] 3.4 规则 1.1 基础程序
- [x] 3.5 规则 1.2 语句
- [x] 3.6 规则 1.3 返回语句
- [x] 3.7 规则 1.4 函数输入
- [x] 3.8 规则 1.5 函数输出
- [x] 3.9 规则 2.0 变量声明
- [x] 3.10 规则 2.1 变量声明语句（shadowing）
- [x] 3.11 规则 2.2 赋值语句
- [x] 3.12 规则 3.1 基本表达式
- [x] 3.13 规则 3.2 比较运算
- [x] 3.14 规则 3.3 加减运算
- [x] 3.15 规则 3.4 乘除运算
- [x] 3.16 规则 3.5 函数调用
- [x] 3.17 规则 4.1 if 选择结构
- [x] 3.18 规则 5.0~5.1 循环语句（while）
- [x] 3.19 规则 5.2~5.4 for / loop / break / continue

### 阶段 4: 中间代码生成
- [x] 4.1 Quadruple 数据结构
- [x] 4.2 IRGenerator 类
- [x] 4.3 常量与变量引用 IR
- [x] 4.4 算术表达式 IR
- [x] 4.5 比较表达式 IR
- [x] 4.6 赋值语句 IR（含引用/解引用）
- [x] 4.7 控制流 IR（if/else JZ+JUMP+LABEL）
- [x] 4.8 循环 IR（while/for/loop 回边）
- [x] 4.9 函数调用 IR（PARAM/CALL/RETURN）
- [x] 4.10 函数定义 IR 框架（FUNC_BEGIN/FUNC_END）
- [x] 4.11 四元式格式化输出
- [x] 4.12 CLI 参数 --semantic-output / --ir-output

### 阶段 5: 集成测试
- [x] 5.1 综合正确用例（test_full.rs：30+ 函数）
- [x] 5.2 语义错误测试（test_errors.rs：17 函数 / 18 错误）
- [x] 5.3 批量测试脚本（semantic_tests.sh：22/22 通过）
- [x] 5.4 Linux + Windows(wine) 交叉验证通过

### 阶段 6: 选做规则
- [x] 6.1 else / else if
- [x] 6.2 for / loop / break / continue
- [x] 6.3 不可变变量 / 引用 / 借用（含借用追踪）
- [x] 6.4 数组（类型、表达式、元素访问）
- [x] 6.5 元组（类型、表达式、元素访问）

### 阶段 7: 文档
- [ ] 7.1 设计报告
- [ ] 7.2 报告 PPT
