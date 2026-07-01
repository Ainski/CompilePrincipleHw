---
description: 作业3 里程碑工作流通用规则，所有里程碑共享
---
# 里程碑通用规则（作业3）

- 每完成一项任务，立即更新对应的 `milestones/M{N}.md`，不等批量更新
- 每项任务完成后必须自测，并在里程碑文件中记录测试方式和结果，**不允许无测试记录的 `[x]`**
- 遵守 `.claude/CLAUDE.md` 路线图中的跨里程碑约束
- 当里程碑全部完成时，更新 `.claude/CLAUDE.md` 路线图（标记 ✅，指向下一里程碑）并告知用户
- 不跳过当前里程碑去提前做后续里程碑的任务
- 发现任务清单遗漏必要步骤，先添加再执行，不默默跳过
- 标记 `[!]` 的任务需用户确认后才能继续推进
- 测试遗留问题必须标注，不默默跳过失败的测试

## 本项目测试方式
- **集成测试**（优先）：`bash parser/testfiles/semantic_tests.sh [parser路径]`，按 exit code 判定 pass/fail
- **构建验证**（最低）：Docker `docker build` 或本地 `./run.sh` 编译通过
- **手动验证**：`./parser --input <file> --ir-output -` 或 `--asm-output` 检查输出
- **IR golden 对比**：重大重构后，与 `test_full_ir.txt` 逐行 diff 确认行为不变
