# CompilationPricinpalBigHw
编译原理课程大作业仓库
程浩然 黄艺鑫

## 使用方法

```bash
sudo ./run.sh #构建并运行
sudo ./run.sh --clean #清理生成文件
sudo ./run.sh --help  # 查看使用说明
```
项目当中的输入输出路径均可调整，具体请查看[run.sh](./run.sh)

```bash
# 定义文件路径变量（不依赖外部命令）
flex_file="main.l"               # flex源文件
flex_output="lex.yy.c"           # flex生成的C文件
flex_input_file="input.c"        # 测试输入文件
flex_output_executable="clex"     # 最终可执行文件
test_output="output.txt"          # 测试输出文件
```

## 更多帮助
1. lex所提供的接口为一个csv文件，请你阅读[promised_lex_output.md](./docs/promised_lex_output.md)