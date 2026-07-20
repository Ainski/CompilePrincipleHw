// ============================================================
//  中间代码生成测试 — 覆盖所有 IR 指令类型
// ============================================================

// ---- 算术运算 (ADD, SUB, MUL, DIV) ----
fn ir_arithmetic(mut a:i32) -> i32 {
    let b=a+1;
    let c=a-2;
    let d=a*3;
    let e=a/4;
    return b+c-d;
}

// ---- 比较运算 (EQ, NE, LT, LE, GT, GE) ----
fn ir_comparisons(mut a:i32, mut b:i32) {
    let c=a<b;
    let d=a<=b;
    let e=a>b;
    let f=a>=b;
    let g=a==b;
    let h=a!=b;
}

// ---- if/else (JZ, JUMP, LABEL) ----
fn ir_if_else(mut x:i32) -> i32 {
    if x>0 {
        return 1;
    } else {
        return 0;
    }
}

// ---- 嵌套 if ----
fn ir_nested_if(mut x:i32) -> i32 {
    if x>10 {
        if x>20 {
            return 2;
        }
        return 1;
    }
    return 0;
}

// ---- while 循环 (JZ, JUMP, LABEL) ----
fn ir_while(mut n:i32) {
    while n>0 {
        n=n-1;
    }
}

// ---- for 循环 (LT, ADD, JZ, JUMP) ----
fn ir_for(mut n:i32) {
    for mut i in 1..n {
        n=n-1;
    }
}

// ---- loop + break + continue ----
fn ir_break_continue() {
    loop {
        break;
    }
}

fn ir_while_continue() {
    while 1==0 {
        continue;
    }
}

// ---- 不可变引用 (REF) ----
fn ir_immut_ref(mut a:i32) {
    let b:& i32=&a;
}

// ---- 可变引用 + 解引用 (REF, DEREF) ----
fn ir_mut_ref(mut a:i32) {
    let mut c:&mut i32=&mut a;
    *c=10;
}

// ---- 数组操作 (ARRAY_LIT, INDEX_LOAD, INDEX_STORE) ----
fn ir_arrays(mut arr:[i32;3]) {
    arr=[1,2,3];
    let x:i32=arr[0];
    arr[1]=x;
}

// ---- 函数调用 (PARAM, CALL, RETURN) ----
fn ir_double(mut x:i32) -> i32 {
    return x*2;
}

fn ir_main() -> i32 {
    let mut a:i32=5;
    let mut b:i32=ir_double(a);
    return b;
}
#