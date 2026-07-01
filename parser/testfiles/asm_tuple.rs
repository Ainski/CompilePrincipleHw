fn tuple_test(mut n: i32) -> i32 {
    let mut t: (i32, i32, i32);
    t = (n, n + 1, n + 2);
    return t.0 + t.1 + t.2;
}
