fn block_test(mut x: i32) -> i32 {
    let z = { let t = x + 1; t };
    let b = if x > 0 { 100 } else { 200 };
    return z + b;
}
