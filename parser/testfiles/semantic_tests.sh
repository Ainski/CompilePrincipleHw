#!/bin/bash
# 语义分析和中间代码集成测试脚本
# 用法: bash semantic_tests.sh [parser_path]
# 在 Docker 中: bash semantic_tests.sh ./build/bin/parser

PARSER="${1:-./build/bin/parser}"
PASS=0
FAIL=0
TOTAL=0

run_test() {
    local name="$1"
    local input="$2"
    local expect_pass="$3"  # "pass" 或 "fail"

    TOTAL=$((TOTAL + 1))
    output=$("$PARSER" --input "$input" -q 2>&1)
    exit_code=$?

    if [ "$expect_pass" = "pass" ]; then
        if [ $exit_code -eq 0 ]; then
            PASS=$((PASS + 1))
            echo "  ✓ $name"
        else
            FAIL=$((FAIL + 1))
            echo "  ✗ $name (expected pass, got errors)"
            echo "    $output" | head -3
        fi
    else
        if [ $exit_code -ne 0 ]; then
            PASS=$((PASS + 1))
            echo "  ✓ $name (error detected as expected)"
        else
            FAIL=$((FAIL + 1))
            echo "  ✗ $name (expected error, but passed)"
        fi
    fi
}

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

echo "=========================================="
echo "  Semantic Analysis Integration Tests"
echo "=========================================="

# ---- Rule 1.x: Program structure ----
echo ""
echo "[Rule 1.x] Program structure"

cat > "$TMPDIR/r1_1.rs" << 'EOF'
fn program_1_1() {

}
#
EOF
run_test "1.1 basic program" "$TMPDIR/r1_1.rs" "pass"

cat > "$TMPDIR/r1_2.rs" << 'EOF'
fn program_1_2() {
    ;;;;;
}
#
EOF
run_test "1.2 statements" "$TMPDIR/r1_2.rs" "pass"

cat > "$TMPDIR/r1_3.rs" << 'EOF'
fn program_1_3() {
    return ;
}
#
EOF
run_test "1.3 return statement" "$TMPDIR/r1_3.rs" "pass"

cat > "$TMPDIR/r1_4.rs" << 'EOF'
fn program_1_4(mut a:i32) {

}
#
EOF
run_test "1.4 function input" "$TMPDIR/r1_4.rs" "pass"

cat > "$TMPDIR/r1_5_pass.rs" << 'EOF'
fn program_1_5() -> i32 {
    return 1;
}
#
EOF
run_test "1.5 function output (correct)" "$TMPDIR/r1_5_pass.rs" "pass"

cat > "$TMPDIR/r1_5_fail1.rs" << 'EOF'
fn program_1_5_fail1() -> i32 {
    return ;
}
#
EOF
run_test "1.5 return void in i32 function (error)" "$TMPDIR/r1_5_fail1.rs" "fail"

cat > "$TMPDIR/r1_5_fail2.rs" << 'EOF'
fn program_1_5_fail2() {
    return 1;
}
#
EOF
run_test "1.5 return i32 in void function (error)" "$TMPDIR/r1_5_fail2.rs" "fail"

# ---- Rule 2.x: Variable declaration and assignment ----
echo ""
echo "[Rule 2.x] Variable declaration and assignment"

cat > "$TMPDIR/r2_pass.rs" << 'EOF'
fn program_2(mut a:i32) {
    let mut b:i32;
    let mut c=1;
    let mut d:i32=2;
    a=32;
    b=a;
}
#
EOF
run_test "2.x variable decl+assign (correct)" "$TMPDIR/r2_pass.rs" "pass"

cat > "$TMPDIR/r2_undeclared.rs" << 'EOF'
fn program_2_err() {
    a=32;
}
#
EOF
run_test "2.2 assign to undeclared var (error)" "$TMPDIR/r2_undeclared.rs" "fail"

cat > "$TMPDIR/r2_unassigned.rs" << 'EOF'
fn program_2_err2() {
    let mut a:i32;
    let mut b:i32=a;
}
#
EOF
run_test "2.2 use unassigned var (error)" "$TMPDIR/r2_unassigned.rs" "fail"

cat > "$TMPDIR/r2_immutable.rs" << 'EOF'
fn program_2_err3() {
    let a:i32=1;
    a=2;
}
#
EOF
run_test "6.1 assign to immutable var (error)" "$TMPDIR/r2_immutable.rs" "fail"

# ---- Rule 3.x: Expressions ----
echo ""
echo "[Rule 3.x] Expressions"

cat > "$TMPDIR/r3_pass.rs" << 'EOF'
fn program_3(mut a:i32) -> i32 {
    0;
    (1);
    a;
    1+2;
    3-4;
    1*2;
    3/4;
    1<2;
    3>4;
    5<=6;
    7>=8;
    9==10;
    11!=12;
    return a+1;
}
#
EOF
run_test "3.x all expressions (correct)" "$TMPDIR/r3_pass.rs" "pass"

# ---- Rule 3.5: Function calls ----
echo ""
echo "[Rule 3.5] Function calls"

cat > "$TMPDIR/r3_5_pass.rs" << 'EOF'
fn helper() {

}
fn program_3_5() {
    helper();
}
#
EOF
run_test "3.5 function call (correct)" "$TMPDIR/r3_5_pass.rs" "pass"

cat > "$TMPDIR/r3_5_arity.rs" << 'EOF'
fn helper2() {

}
fn program_3_5_err() {
    helper2(1);
}
#
EOF
run_test "3.5 wrong argument count (error)" "$TMPDIR/r3_5_arity.rs" "fail"

# ---- Rule 4.x: Selection ----
echo ""
echo "[Rule 4.x] Selection"

cat > "$TMPDIR/r4_pass.rs" << 'EOF'
fn program_4(mut a:i32) -> i32 {
    if a>0 {
        return 1;
    }
    if a>0 {
        return 1;
    } else {
        return 0;
    }
    if a>0 {
        return 1;
    } else if a<0 {
        return 2;
    } else {
        return 0;
    }
}
#
EOF
run_test "4.x if/else/else-if (correct)" "$TMPDIR/r4_pass.rs" "pass"

# ---- Rule 5.x: Loops ----
echo ""
echo "[Rule 5.x] Loops"

cat > "$TMPDIR/r5_pass.rs" << 'EOF'
fn program_5(mut n:i32) {
    while n>0 {
        n=n-1;
    }
    for mut i in 1..n+1 {
        n=n-1;
    }
    loop {
        break;
    }
    while 1==0 { continue; }
}
#
EOF
run_test "5.x while/for/loop/break/continue (correct)" "$TMPDIR/r5_pass.rs" "pass"

cat > "$TMPDIR/r5_break_err.rs" << 'EOF'
fn program_5_err() {
    break;
}
#
EOF
run_test "5.4 break outside loop (error)" "$TMPDIR/r5_break_err.rs" "fail"

cat > "$TMPDIR/r5_cont_err.rs" << 'EOF'
fn program_5_err2() {
    continue;
}
#
EOF
run_test "5.4 continue outside loop (error)" "$TMPDIR/r5_cont_err.rs" "fail"

# ---- Rule 6.x: References ----
echo ""
echo "[Rule 6.x] References"

cat > "$TMPDIR/r6_pass.rs" << 'EOF'
fn program_6(mut a:i32) {
    let b:& i32=&a;
    let mut c:&mut i32=&mut a;
}
#
EOF
run_test "6.x references (correct)" "$TMPDIR/r6_pass.rs" "pass"

cat > "$TMPDIR/r6_deref_err.rs" << 'EOF'
fn program_6_err(mut a:i32) {
    let mut b=*a;
}
#
EOF
run_test "6.4 deref non-reference (error)" "$TMPDIR/r6_deref_err.rs" "fail"

# ---- Rule 8.x: Arrays ----
echo ""
echo "[Rule 8.x] Arrays"

cat > "$TMPDIR/r8_pass.rs" << 'EOF'
fn program_8(mut a:[i32;3]) {
    let mut b:[i32;3];
    a=[1,2,3];
    let mut c:i32=a[0];
    a[0]=1;
}
#
EOF
run_test "8.x arrays (correct)" "$TMPDIR/r8_pass.rs" "pass"

# ---- Rule 9.x: Tuples ----
echo ""
echo "[Rule 9.x] Tuples"

cat > "$TMPDIR/r9_pass.rs" << 'EOF'
fn program_9(mut a:(i32,i32)) {
    a=(1,2);
}
#
EOF
run_test "9.x tuples (correct)" "$TMPDIR/r9_pass.rs" "pass"

# ---- Summary ----
echo ""
echo "=========================================="
echo "  Results: $PASS/$TOTAL passed, $FAIL failed"
echo "=========================================="

[ $FAIL -eq 0 ] && exit 0 || exit 1
