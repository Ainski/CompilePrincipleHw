fn sum_to(mut n: i32) -> i32 {
    let mut s: i32 = 0;
    let mut i: i32 = 1;
    while i <= n {
        s = s + i;
        i = i + 1;
    }
    return s;
}
