fn test_7_1(mut a: i32) -> i32 {
    let z = { let t = a + 1; t };
    return z;
}

fn test_7_2(mut a: i32) -> i32 {
    let t = a * 2;
    t
}

fn test_7_3(mut a: i32) -> i32 {
    let b = if a > 0 { 1 } else { 0 };
    return b;
}

fn test_7_4() -> i32 {
    let x = loop { break 5; };
    return x;
}

fn test_7_5(mut n: i32) -> i32 {
    let s = if n > 0 {
        let a = n + n;
        a
    } else {
        0
    };
    return s;
}
