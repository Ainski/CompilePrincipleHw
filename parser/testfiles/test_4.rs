// 规则 4.1~4.3 选择结构
fn program_4_1(a:i32) -> i32 {
    if a>0 {
        return 1;
    }
}
fn program_4_2(a:i32) -> i32 {
    if a>0 {
        return 1;
    } else {
        return 0;
    }
}

fn program_4_3(a:i32) -> i32 {
    if a>0 {
        return a+1;
    } else if a<0 {
        return a-1;
    } else {
        return 0;
    }
}
#
