#!/bin/bash
# build environment
# please run this shell with sudo 

# 修复：变量赋值=两边无空格
flex_version=$(flex --version | awk '{print $2}')  # 提取版本号（如2.6.4）
flex_file="main.l"               # flex源文件
flex_output="lex.yy.c"           # flex生成的c文件
flex_input_file="input.c"        # 测试输入文件
flex_output_executable="clex"    # 最终可执行文件
test_output="output.txt"            # 测试输出文件
# 检查flex版本
if [ "$flex_version" = "2.6.4" ]; then 
    echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : flex 2.6.4 is already installed"
else 
    echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : flex $flex_version is not installed, installing flex 2.6.4..."
    apt install -y flex  # 加-y自动确认安装，避免交互
fi

# 进入flex目录并执行编译+运行（修复核心执行语句）
if [ -d "flex" ]; then  # 先检查flex目录是否存在，避免cd失败
    cd flex || exit 1   # cd失败则退出脚本，防止后续错误
    
    # 1. 用flex生成C代码
    echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 生成flex代码: $flex_output"
    flex -o "$flex_output" "$flex_file"
    
    # 2. 用gcc编译生成可执行文件（-lfl链接flex库）
    echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 编译可执行文件: $flex_output_executable"
    gcc -o "$flex_output_executable" "$flex_output" -lfl
    
    # 3. 执行可执行文件，并重定向输入（核心执行语句）
    if [ -f "$flex_output_executable" ]; then  # 检查可执行文件是否生成成功
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 执行程序，输入文件: $flex_input_file"
        ./$flex_output_executable < "$flex_input_file" > "$test_output"  # 正确写法：./文件名 无空格
    else
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 错误：可执行文件 $flex_output_executable 未生成"
        exit 1
    fi
    
    cd ..  # 回到上级目录
else
    echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 错误：flex目录不存在"
    exit 1
fi

echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 执行完成"