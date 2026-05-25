// 1.5 函数返回类型不一致
fn err_return_1() -> i32 {
    return ;
}

fn err_return_2() {
    return 1;
}

// 2.2 赋值给未声明变量
fn err_undeclared() {
    a=32;
}

// 2.2 赋值类型不一致
fn err_type_mismatch(mut a:i32) {
    a=1==1;
}

// 2.2 使用未赋值变量
fn err_unassigned() {
    let mut a:i32;
    let mut b:i32=a;
}

// 6.1 不可变变量二次赋值
fn err_immutable_assign() {
    let a:i32=1;
    a=2;
}

// 3.5 函数调用参数数量不一致
fn err_call_arity_helper() {

}
fn err_call_arity() {
    err_call_arity_helper(1);
}

// 5.4 break 不在循环中
fn err_break_outside() {
    break;
}

// 5.4 continue 不在循环中
fn err_continue_outside() {
    continue;
}
#
