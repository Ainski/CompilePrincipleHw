// this is a single line comment
/* this is a block comment  */

fn main() -> i32 {
    let mut x: i32 = 10;
    let y: i32 = 20;
    let z: i32 = 30;

    // test if-else
    if x > y {
        x = x + 1;
    } else if x < y {
        x = x - 1;
    } else {
        x = 0;
    }

    // test while loop
    while x < 15 {
        x = x + 1;
    }
    // test for-in loop
    for i in 0..10 {
        if i == 5 {
            break;
        }
        continue;
    }

    // test loop loop
    loop {
        x = x + 1;
        if x >= 20 {
            break;
        }
    }

    // test operators
    let a: i32 = x + y;
    let b: i32 = x - y;
    let c: i32 = x * y;
    let d: i32 = x / y;
    
    // test comparison operators
    let e: bool = x == y;
    let f: bool = x != y;
    let g: bool = x >= y;
    let h: bool = x <= y;

    // test arrays
    let arr: [i32; 3] = [1, 2, 3];
    let val: i32 = arr[0];

    // test function calls
    let result: i32 = add(x, y);

    // test return
    return result;
}

fn add(a: i32, b: i32) -> i32 {
    let sum: i32 = a + b;
    return sum;
}

fn test_special() -> i32 {
    let x: i32 = 100;
    let y: i32 = x;
    let z: i32 = y;
    
    // test . and ..
    let range = 0..100;
    
    return x;
}

#
