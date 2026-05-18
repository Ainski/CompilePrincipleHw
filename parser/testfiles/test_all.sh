#!/bin/bash
# 批量运行测试用例并生成词法/语法分析结果
# 使用方法: cd parser && bash test_all.sh

PARSER="./build/bin/parser"
TEST_DIR="./testfiles"

if [ ! -f "$PARSER" ]; then
    echo "错误: 未找到可执行文件 $PARSER"
    echo "请先运行 sudo ./run.sh 构建"
    exit 1
fi

pass=0
fail=0

for f in "$TEST_DIR"/test_*.rs; do
    name=$(basename "$f" .rs)
    echo "===== 测试: $name ====="

    "$PARSER" -q --input "$f" \
        --lexer-output "$TEST_DIR/${name}_lexer.tsv" \
        --parser-output "$TEST_DIR/${name}_parse.txt"

    if [ $? -eq 0 ]; then
        echo "[PASS] $name"
        pass=$((pass+1))
    else
        echo "[FAIL] $name"
        fail=$((fail+1))
    fi
    echo ""
done

echo "===== 测试结果 ====="
echo "通过: $pass"
echo "失败: $fail"
echo "总计: $((pass+fail))"
