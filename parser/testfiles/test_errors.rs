// ============================================================
// 大作业2 错误检测测试 - 覆盖 assignment2.md 中标注的错误场景
// 每个 error_ 函数应恰好产生一个语义错误
// ============================================================

// ---- 1.5 返回类型不一致 ----
// assignment2.md: "返回语句的类型（空）和函数声明返回类型（i32）不一致"
fn err_1_5a() -> i32 {
    return ;
}

// assignment2.md: "返回语句的类型（i32）和函数声明返回类型（空）不一致"
fn err_1_5b() {
    return 1;
}

// ---- 2.2 赋值给未声明变量 ----
// assignment2.md: "变量未声明"
fn err_2_2a() {
    a=32;
}

// ---- 2.2 赋值类型不一致 ----
// assignment2.md: "变量类型不一致"
fn err_2_2b(mut a:i32) {
    a=1==1;
}

// ---- 2.2 右值未提前声明 ----
// assignment2.md: "右值未提前声明"
fn err_2_2c() {
    let mut b:i32=a;
}

// ---- 2.2 右值未提前赋值 ----
// assignment2.md: "右值未提前赋值"
fn err_2_2d() {
    let mut a:i32;
    let mut b:i32=a;
}

// ---- 6.1 不可变变量不可二次赋值 ----
// assignment2.md: "不可变变量不可被二次赋值"
fn err_6_1() {
    let c:i32=1;
    c=2;
}

// ---- 3.5 函数调用：实参数量不一致 ----
// assignment2.md: "实参数量与形参数量不一致"
fn err_3_5a_helper() {

}
fn err_3_5a() {
    err_3_5a_helper(1);
}

// ---- 3.5 函数调用：无返回值函数不能作为右值 ----
// assignment2.md: "无返回值函数不能作为右值"
fn err_3_5b_helper() {

}
fn err_3_5b() {
    let mut a=err_3_5b_helper();
}

// ---- 3.5 函数调用：实参类型不一致 ----
// assignment2.md: "实参类型与形参类型不一致"
fn err_3_5c_helper(mut a:i32) {

}
fn err_3_5c() {
    err_3_5c_helper(1==1);
}

// ---- 5.4 break 不在循环体内 ----
// assignment2.md: "break; 必须出现在循环体内"
fn err_5_4a() {
    break;
}

// ---- 5.4 continue 不在循环体内 ----
// assignment2.md: "continue; 必须出现在循环体内"
fn err_5_4b() {
    continue;
}

// ---- 6.3 可变引用不能与不可变引用共存 ----
// assignment2.md: "可变引用不能和其他的引用共存"
fn err_6_3a() {
    let mut a:i32=1;
    let b=&a;
    let mut c=&mut a;
}

// ---- 6.3 仅允许从可变变量创建可变引用 ----
// assignment2.md: "仅支持从可变变量创建可变引用"
fn err_6_3b() {
    let a:i32=1;
    let mut b=&mut a;
}

// ---- 6.4 不允许对非引用类型解引用 ----
// assignment2.md: "不允许对非引用类型变量进行解引用"
fn err_6_4a() {
    let mut a:i32=1;
    let mut b=*a;
}

// ---- 6.4 不可变引用不可以修改指向数据 ----
// assignment2.md: "不可变引用不可以修改指向数据"
fn err_6_4b() {
    let mut a:i32=1;
    let mut b=&a;
    *b=2;
}

// ---- 8.2 数组初始化元素数量不一致 ----
// assignment2.md: "初始化时的元素数量与数组长度不一致"
fn err_8_2a(mut a:i32) {
    let mut a:[i32;2];
    a=[1,2,3];
}

// ---- 9.2 元组初始化元素数量不一致 ----
// assignment2.md: "初始化时的元素数量与元组长度不一致"
fn err_9_2a(mut a:i32) {
    let mut a:(i32,i32);
    a=(1,2,3);
}
#
