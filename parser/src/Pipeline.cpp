#include "../include/Pipeline.h"

// ============================================================
//  Pipeline —— 编译器流水线协调器（作业3 a' 嫁接合并）
//
//  SemanticAnalyzer 已嫁接 IR 生成（visitX 在语义检查同时 emit 四元式），
//  故 run() 只需一次 analyze，即完成「语义分析 + 中间代码生成」单次遍历。
// ============================================================

void Pipeline::run(const Node* root) {
    if (!root) return;
    sem_.analyze(root);   // a' 单次遍历：语义检查 + IR 生成
}
