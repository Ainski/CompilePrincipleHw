#ifndef PIPELINE_H
#define PIPELINE_H

#include "SemanticAnalyzer.h"
#include "parser.h"
#include <unordered_map>
#include <string>

// ============================================================
//  NodeResult —— 节点遍历结果（a' 三钩子模型的数据载体）
//
//  表达式节点：type 与 ir_temp 有效（求值结果）
//  语句节点  ：均为空（处理靠副作用：修改符号表、emit 四元式）
// ============================================================
struct NodeResult {
    STypePtr type;       // 表达式的语义类型
    std::string ir_temp; // 表达式求值结果的 IR 临时变量名
};

// ============================================================
//  Pipeline —— 编译器流水线协调器（作业3 a' 架构合并）
//
//  设计目标：driver（Pipeline）接管 AST 遍历，SemanticAnalyzer 与
//  IRGenerator 的处理逻辑改造为三钩子（onEnter/process/onLeave）非递归版，
//  实现单次后序遍历，函数体内从双遍历变单遍历。
//
//  当前阶段 A：Pipeline 作为新流水线入口，run() 内部兼容调用旧
//  analyze/generate，行为与改造前完全一致。traverse 与三钩子为骨架，
//  待阶段 B+ 填充。
// ============================================================
class Pipeline {
    SemanticAnalyzer& sem_;
    std::unordered_map<const Node*, NodeResult> results_;  // 阶段 B 骨架（未启用）

public:
    explicit Pipeline(SemanticAnalyzer& sem) : sem_(sem) {}

    // 流水线入口：语义分析（嫁接合并后同时生成 IR）
    // 语义失败时由 sem_.hasErrors() 判定，由调用方（main）处理
    void run(const Node* root);

    // 访问内部状态
    SemanticAnalyzer& sem() { return sem_; }
};

#endif
