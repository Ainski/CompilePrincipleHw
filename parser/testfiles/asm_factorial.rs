fn factorial(mut n: i32) -> i32 {
    let mut r: i32 = 1;
    let mut i: i32 = 1;
    while i <= n {
        r = r * i;
        i = i + 1;
    }
    return r;
}
