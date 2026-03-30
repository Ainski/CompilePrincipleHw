// 这是一个单行注释测试
/* 这是一个多行注释测试 */

fn main() -> i32 {
    let mut x: i32 = 10;
    let y: i32 = 20;
    let z: i32 = 30;

    // 测试 if-else
    if x > y {
        x = x + 1;
    } else if x < y {
        x = x - 1;
    } else {
        x = 0;
    }

    // 测试 while 循环
    while x < 15 {
        x = x + 1;
    }

    // 测试 for-in 循环
    for i in 0..10 {
        if i == 5 {
            break;
        }
        continue;
    }

    // 测试 loop 循环
    loop {
        x = x + 1;
        if x >= 20 {
            break;
        }
    }

    // 测试运算符
    let a: i32 = x + y;
    let b: i32 = x - y;
    let c: i32 = x * y;
    let d: i32 = x / y;
    
    // 测试比较运算符
    let e: bool = x == y;
    let f: bool = x != y;
    let g: bool = x >= y;
    let h: bool = x <= y;

    // 测试数组
    let arr: [i32; 3] = [1, 2, 3];
    let val: i32 = arr[0];

    // 测试函数调用
    let result: i32 = add(x, y);

    // 测试 return
    return result;
}

fn add(a: i32, b: i32) -> i32 {
    let sum: i32 = a + b;
    return sum;
}

// 测试特殊符号
fn test_special() -> i32 {
    let x: i32 = 100;
    let y: i32 = x;
    let z: i32 = y;
    
    // 测试 . 和 ..
    let range = 0..100;
    
    return x;
}

#
