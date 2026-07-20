// ============================================================
//  语义分析错误检测测试 — 覆盖所有错误场景
//  每个函数应恰好产生一个语义错误
// ============================================================

// ---- 1.5 函数返回类型不一致 ----
fn err_return_1() -> i32 {
    return ;
}

fn err_return_2() {
    return 1;
}

// ---- 2.2 赋值给未声明变量 ----
fn err_undeclared() {
    a=32;
}

// ---- 2.2 赋值类型不一致 ----
fn err_type_mismatch(mut a:i32) {
    a=1==1;
}

// ---- 2.2 使用未赋值变量 ----
fn err_unassigned() {
    let mut a:i32;
    let mut b:i32=a;
}

// ---- 6.1 不可变变量二次赋值 ----
fn err_immutable_assign() {
    let a:i32=1;
    a=2;
}

// ---- 3.5 函数调用参数数量不一致 ----
fn err_call_arity_helper() {

}
fn err_call_arity() {
    err_call_arity_helper(1);
}

// ---- 3.5 无返回值函数不能作为右值 ----
fn err_void_rvalue_helper() {

}
fn err_void_rvalue() {
    let mut a=err_void_rvalue_helper();
}

// ---- 3.5 实参类型与形参类型不一致 ----
fn err_arg_type_helper(mut a:i32) {

}
fn err_arg_type() {
    err_arg_type_helper(1==1);
}

// ---- 5.4 break 不在循环中 ----
fn err_break_outside() {
    break;
}

// ---- 5.4 continue 不在循环中 ----
fn err_continue_outside() {
    continue;
}

// ---- 6.3 可变引用不能与不可变引用共存 ----
fn err_mut_ref_conflict() {
    let mut a:i32=1;
    let b=&a;
    let mut c=&mut a;
}

// ---- 6.3 仅允许从可变变量创建可变引用 ----
fn err_mut_ref_from_immut() {
    let a:i32=1;
    let mut b=&mut a;
}

// ---- 6.4 不允许对非引用类型解引用 ----
fn err_deref_non_ref() {
    let mut a:i32=1;
    let mut b=*a;
}

// ---- 6.4 不可变引用不可以修改指向数据 ----
fn err_write_immut_ref() {
    let mut a:i32=1;
    let mut b=&a;
    *b=2;
}

// ---- 8.2 数组初始化元素数量不一致 ----
fn err_array_init_count(mut a:i32) {
    let mut a:[i32;2];
    a=[1,2,3];
}

// ---- 9.2 元组初始化元素数量不一致 ----
fn err_tuple_init_count(mut a:i32) {
    let mut a:(i32,i32);
    a=(1,2,3);
}
#
