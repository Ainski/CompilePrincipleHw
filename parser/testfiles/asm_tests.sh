#!/bin/bash
# 作业3 M4-测试：目标代码（x86-64 汇编）运行验证
# 用法: bash testfiles/asm_tests.sh [parser_path]
# 逐程序生成 .s → as → ld → 运行，校验 exit code == 预期返回值

PARSER="${1:-./build/bin/parser}"
PASS=0; FAIL=0; TOTAL=0
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

run_asm() {
    local name="$1" src="$2" entry="$3" arg="$4" expected="$5"
    TOTAL=$((TOTAL + 1))
    "$PARSER" --input "$src" --asm-output "$TMPDIR/t.s" --entry "$entry" --entry-arg "$arg" -q 2>/dev/null
    if [ $? -ne 0 ]; then echo "  ✗ $name (parser/语义失败)"; FAIL=$((FAIL+1)); return; fi
    if ! as "$TMPDIR/t.s" -o "$TMPDIR/t.o" 2>/dev/null; then
        echo "  ✗ $name (汇编 as 失败)"; FAIL=$((FAIL+1)); return; fi
    if ! ld "$TMPDIR/t.o" -o "$TMPDIR/t.bin" 2>/dev/null; then
        echo "  ✗ $name (链接 ld 失败)"; FAIL=$((FAIL+1)); return; fi
    "$TMPDIR/t.bin"; local got=$?
    if [ "$got" = "$expected" ]; then
        PASS=$((PASS+1)); echo "  ✓ $name($arg) = $got"
    else
        FAIL=$((FAIL+1)); echo "  ✗ $name($arg) 期望 $expected 实际 $got"
    fi
}

echo "=========================================="
echo "  目标代码运行测试（x86-64 汇编执行）"
echo "=========================================="

echo ""
echo "[算术 + 控制流]"
run_asm "fibonacci"   testfiles/asm_fib.rs       fibonacci  10  55
run_asm "fibonacci"   testfiles/asm_fib.rs       fibonacci  7   13
run_asm "factorial"   testfiles/asm_factorial.rs factorial  5   120
run_asm "factorial"   testfiles/asm_factorial.rs factorial  4   24
run_asm "compute"     testfiles/asm_arith.rs     compute    5   11
run_asm "sum_to"      testfiles/asm_loop.rs      sum_to     10  55
run_asm "sum_to"      testfiles/asm_loop.rs      sum_to     5   15

echo ""
echo "[7.x 表达式块]"
run_asm "block+if表达式" testfiles/asm_block.rs  block_test 5   106
run_asm "block+if表达式" testfiles/asm_block.rs  block_test 0   201

echo ""
echo "[9.3 元组元素访问]"
run_asm "元组元素 a.0/a.1/a.2" testfiles/asm_tuple.rs  tuple_test 5   18
run_asm "元组元素 a.0/a.1/a.2" testfiles/asm_tuple.rs  tuple_test 10  33

echo ""
echo "=========================================="
echo "  结果: $PASS/$TOTAL 通过, $FAIL 失败"
echo "=========================================="
[ $FAIL -eq 0 ] && exit 0 || exit 1
