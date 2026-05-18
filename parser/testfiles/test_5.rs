// 规则 5.0~5.4 循环结构
fn program_5_1(mut n:i32) {
    while n>0 {
        n=n-1;
    }
}
fn program_5_2(mut n:i32) {
    for mut i in 1..n+1 {
        n=n-1;
    }
}

fn program_5_3() {
    loop {

    }
}
fn program_5_4() {
    while 1==0 { continue; }
    while 1==1 { break; }
}
#
