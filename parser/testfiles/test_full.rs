// ============================================================
// 大作业2 综合测试 - 覆盖全部必做+选做语义规则
// 正确用例：应通过语义分析
// ============================================================

// ---- 1.1 基础程序 ----
fn test_1_1() {

}

// ---- 1.2 语句 ----
fn test_1_2() {
    ;;;;;
}

// ---- 1.3 返回语句 ----
fn test_1_3() {
    return ;
}

// ---- 1.4 函数输入 ----
fn test_1_4(mut a:i32, mut b:i32) {

}

// ---- 1.5 函数输出 ----
fn test_1_5() -> i32 {
    return 1;
}

// ---- 2.0~2.1 变量声明（含 shadowing）----
fn test_2_1() {
    let mut a:i32;
    let mut b;
    let mut c:i32=1;
    let mut d=2;
    let mut a:i32;
    a=2;
}

// ---- 2.2 赋值 ----
fn test_2_2(mut a:i32) {
    a=32;
}

// ---- 2.3 变量声明赋值 ----
fn test_2_3() {
    let mut a:i32=1;
    let mut b=2;
    let mut a=3;
    let mut a:i32=4;
}

// ---- 3.1 基本表达式 ----
fn test_3_1(mut a:i32) {
    0;
    (1);
    ((2));
    a;
    (a);
    ((a));
}

// ---- 3.2 比较运算（全部6种）----
fn test_3_2() {
    1<2;
    1<=2;
    1>2;
    1>=2;
    1==2;
    1!=2;
}

// ---- 3.3 加减运算 ----
fn test_3_3() {
    1+2;
    3-4;
}

// ---- 3.4 乘除运算 ----
fn test_3_4() {
    1*2;
    3/4;
}

// ---- 3.5 函数调用 ----
fn test_3_5_helper() {

}
fn test_3_5_helper2(mut a:i32) {

}
fn test_3_5() {
    test_3_5_helper();
    test_3_5_helper2(1+2);
}

// ---- 4.1 if 选择 ----
fn test_4_1(mut a:i32) -> i32 {
    if a>0 {
        return 1;
    }
    return 0;
}

// ---- 4.2 else ----
fn test_4_2(mut a:i32) -> i32 {
    if a>0 {
        return 1;
    } else {
        return 0;
    }
}

// ---- 4.3 else if ----
fn test_4_3(mut a:i32) -> i32 {
    if a>0 {
        return a+1;
    } else if a<0 {
        return a-1;
    } else {
        return 0;
    }
}

// ---- 5.1 while 循环 ----
fn test_5_1(mut n:i32) {
    while n>0 {
        n=n-1;
    }
}

// ---- 5.2 for 循环 ----
fn test_5_2(mut n:i32) {
    for mut i in 1..n+1 {
        n=n-1;
    }
}

// ---- 5.3 loop 循环 ----
fn test_5_3() {
    loop {

    }
}

// ---- 5.4 break/continue ----
fn test_5_4() {
    while 1==0 { continue; }
    while 1==1 { break; }
}

// ---- 6.1 不可变变量 ----
fn test_6_1() {
    let a:i32=1;
    let b=2;
    let c:i32;
}

// ---- 6.2 不可变引用 ----
fn test_6_2(a:i32) {
    let b:& i32=&a;
    let c=&a;
    let d=&a;
}

// ---- 6.3 可变引用 ----
fn test_6_3(mut a:i32) {
    let mut b:&mut i32=&mut a;
}

// ---- 6.4 借用（解引用）----
fn test_6_4(a:&mut i32) {
    let b=*a;
    *a=3;
}

// ---- 8.1 数组类型 ----
fn test_8_1() {
    let mut a:[i32;3];
    let mut b:[[i32;3];3];
}

// ---- 8.2 数组表达式 ----
fn test_8_2(mut a:[i32;3]) {
    a=[1,2,3];
}

// ---- 8.3 数组元素访问 ----
fn test_8_3(mut a:[i32;3]) {
    let mut b:i32=a[0];
    a[0]=1;
}

// ---- 9.1 元组类型 ----
fn test_9_1() {
    let a:(i32,i32);
    let b:(i32,i32,i32);
}

// ---- 9.2 元组表达式 ----
fn test_9_2(mut a:(i32,i32)) {
    a=(1,2);
}

// ---- 综合用例：完整程序 ----
fn fibonacci(mut n:i32) -> i32 {
    if n<=1 {
        return n;
    }
    let mut a:i32=0;
    let mut b:i32=1;
    let mut i:i32=2;
    while i<=n {
        let mut t:i32=a+b;
        a=b;
        b=t;
        i=i+1;
    }
    return b;
}
#
