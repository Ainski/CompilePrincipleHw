---
description: 推进作业3 里程碑中的下一个任务
---
读取 `.claude/CLAUDE.md` 中的"作业3 里程碑路线图"，确认当前里程碑编号 N。
读取 `milestones/M{N}.md` 的任务清单。
找到第一个 `[ ]` 未完成项，执行它。
执行完毕后，按项目类型自测（本项目最低要求：构建验证 + 手动验证；优先 `bash parser/testfiles/semantic_tests.sh` 集成测试）。
在 `milestones/M{N}.md` 中记录：状态 `[x]`、测试方式、测试结果、遗留问题（如有）。
如果当前里程碑全部完成，更新 `.claude/CLAUDE.md` 路线图标记 ✅ 并指向下一个里程碑。
遇到连续 3 次失败或需要人工决策的问题，标记 `[!]` 并停下通知用户。
遵守 `.claude/rules/milestone-rules.md` 与路线图中的跨里程碑约束。
