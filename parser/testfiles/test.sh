#!/bin/bash
# ============================================================
#  全量测试 — 一键运行
#  用法: ./test.sh [path/to/parser]
# ============================================================

TDIR="$(cd "$(dirname "$0")" && pwd)"
PARSER="${1:-${TDIR}/../build/bin/parser}"
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

PASS=0; FAIL=0; TOTAL=0
PP=0; PF=0; PT=0

G='\033[0;32m'; R='\033[0;31m'; C='\033[0;36m'; B='\033[1m'; N='\033[0m'

header() { PP=0; PF=0; PT=0; echo -e "\n${C}${B}$1${N}"; }
psum()   { echo -e "${C}  $1: ${G}$PP passed${N} / ${R}$PF failed${N} / $PT total${N}"; PASS=$((PASS+PP)); FAIL=$((FAIL+PF)); TOTAL=$((TOTAL+PT)); }

run() {
    local name="$1" expect="$2"; shift 2; PT=$((PT+1))
    local ec=0; local out; out=$("$PARSER" "$@" -q 2>&1) || ec=$?
    if [ "$expect" = "pass" ]; then
        if [ $ec -eq 0 ]; then PP=$((PP+1)); echo -e "  ${G}✓${N} $name"
        else PF=$((PF+1)); echo -e "  ${R}✗${N} $name (exit $ec)"; echo "$out" | head -2; fi
    else
        if [ $ec -ne 0 ]; then PP=$((PP+1)); echo -e "  ${G}✓${N} $name"
        else PF=$((PF+1)); echo -e "  ${R}✗${N} $name (should fail)"; fi
    fi
    return 0
}

run_ir() {
    local name="$1" src="$2"; PT=$((PT+1))
    local ir="$TMPDIR/${name}.ir"
    local ec=0; local out; out=$("$PARSER" --input "$src" --ir-output "$ir" -q 2>&1) || ec=$?
    if [ $ec -eq 0 ] && [ -f "$ir" ] && [ -s "$ir" ]; then
        local n; n=$(wc -l < "$ir" 2>/dev/null || echo 0)
        PP=$((PP+1)); echo -e "  ${G}✓${N} $name (${n} quads)"
    else
        PF=$((PF+1)); echo -e "  ${R}✗${N} $name (no IR)"
        [ -n "$out" ] && echo "$out" | head -2
    fi
    return 0
}

[ ! -f "$PARSER" ] && echo -e "${R}Not found: $PARSER${N}" && exit 1
echo -e "${B}Parser:${N} $PARSER"

# ======== Phase 1: Lexer + Parser (numbered test files only) ========
header "Phase 1: Lexer + Parser — test_[0-9]*.rs"
for f in "$TDIR"/test_[0-9]_*.rs "$TDIR"/test_[0-9].rs; do
    [ ! -f "$f" ] && continue
    n="$(basename "$f" .rs)"
    run "$n" pass --input "$f" --lexer-output "$TMPDIR/${n}_lex.tsv" --parser-output "$TMPDIR/${n}_parse.txt"
done
psum "Lexer + Parser"

# ======== Phase 2: Semantic — correct programs ========
header "Phase 2: Semantic Analysis — correct programs"
run "test_full"     pass --input "$TDIR/test_full.rs"
run "ir_tests"      pass --input "$TDIR/ir_tests.rs"
psum "Semantic Correct"

# ======== Phase 3: Semantic — error detection ========
header "Phase 3: Semantic Analysis — error detection"
run "sem_errors"    fail --input "$TDIR/semantic_error_tests.rs"
psum "Semantic Errors"

# ======== Phase 4: IR generation ========
header "Phase 4: IR Generation"
for f in "$TDIR"/test_[0-9]_*.rs "$TDIR"/test_[0-9].rs "$TDIR"/test_full.rs "$TDIR"/ir_tests.rs; do
    [ ! -f "$f" ] && continue
    n="$(basename "$f" .rs)"
    run_ir "$n" "$f"
done
psum "IR Generation"

# ======== Final ========
echo -e "\n${C}${B}=========================================="
echo -e "  FINAL: ${G}$PASS passed${N} / ${R}$FAIL failed${N} / $TOTAL total"
echo -e "==========================================${N}\n"
[ $FAIL -eq 0 ] && exit 0 || exit 1
