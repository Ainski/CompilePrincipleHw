#!/bin/bash
# build environment
# please run this shell with sudo

# 定义文件路径变量（不依赖外部命令）
flex_file="main.l"               # flex源文件
flex_output="lex.yy.c"           # flex生成的C文件
flex_input_file="input.c"        # 测试输入文件
flex_output_executable="clex"     # 最终可执行文件
test_output="output.txt"          # 测试输出文件

# 显示帮助信息
usage() {
    echo "Usage: $0 [OPTION]"
    echo "Build and run a Flex lexical analyzer."
    echo ""
    echo "Options:"
    echo "  --clean    Remove all generated files (lex.yy.c, clex, output.txt)"
    echo "  --help     Display this help message and exit"
    echo ""
    echo "Without options, the script builds and runs the analyzer."
}

# 清理生成的文件
clean() {
    echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : Cleaning generated files..."
    if [ -d "flex" ]; then
        cd flex || exit 1
        rm -f "$flex_output" "$flex_output_executable" "$test_output"
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : Removed: $flex_output, $flex_output_executable, $test_output"
        cd ..
    else
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : flex directory not found, nothing to clean."
    fi
    exit 0
}

# 正常的构建流程
build() {
    # 获取当前flex版本（如果未安装，变量为空）
    flex_version=$(flex --version 2>/dev/null | awk '{print $2}')

    # 检查并安装flex 2.6.4
    if [ "$flex_version" = "2.6.4" ]; then
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : flex 2.6.4 is already installed"
    else
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : flex $flex_version is not installed, installing flex 2.6.4..."
        apt install -y flex
    fi

    # 进入flex目录并执行编译+运行
    if [ -d "flex" ]; then
        cd flex || exit 1

        # 1. 用flex生成C代码
        if [ -f "$flex_file" ]; then
            echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : Found flex file: $flex_file"
        else
            echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 错误：flex文件 $flex_file 未找到"
            exit 1
        fi
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 生成flex代码: $flex_output"
        flex -o "$flex_output" "$flex_file"

        # 2. 用gcc编译生成可执行文件
        if [ -f "$flex_output" ]; then
            echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : flex代码生成成功: $flex_output"
        else
            echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 错误：flex代码 $flex_output 未生成"
            exit 1
        fi
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 编译可执行文件: $flex_output_executable"
        gcc -o "$flex_output_executable" "$flex_output" -lfl

        # 3. 执行可执行文件，并重定向输入
        if [ -f "$flex_output_executable" ]; then
            echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 执行程序，输入文件: $flex_input_file"
            ./"$flex_output_executable" < "$flex_input_file" > "$test_output"
        else
            echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 错误：可执行文件 $flex_output_executable 未生成"
            exit 1
        fi

        cd ..
    else
        echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 错误：flex目录不存在"
        exit 1
    fi

    echo "$(date +"%Y-%m-%d %H:%M:%S [%s]") : 执行完成"
}

# 解析命令行参数
if [ $# -eq 0 ]; then
    # 无参数，执行构建流程
    build
else
    case "$1" in
        --clean)
            clean
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
fi