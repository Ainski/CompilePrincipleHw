fn fibonacci(mut n: i32) -> i32 {
    if n <= 1 { return n; }
    let mut a: i32 = 0;
    let mut b: i32 = 1;
    let mut i: i32 = 2;
    while i <= n {
        let mut t: i32 = a + b;
        a = b;
        b = t;
        i = i + 1;
    }
    return b;
}
