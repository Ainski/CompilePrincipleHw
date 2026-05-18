// 规则 6.1~6.4 引用与借用
fn program_6_1() {
    let a:i32;
    let b;
    let c:i32=1;
    let d=2;
}

fn program_6_2(a:i32) {
    let b:& i32=&a;
}

fn program_6_3(mut a:i32) {
    let mut b:&mut i32=&mut a;
}

fn program_6_4(a:&mut i32) {
    let b=*a;
    *a=3;
}
#
