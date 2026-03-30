#!/bin/bash
# build environment
# please run this shell with sudo

# 定义文件路径变量（不依赖外部命令）
flex_file="main.l"               # flex源文件
flex_output="lex.yy.c"           # flex生成的C文件
flex_input_file="input.rs"        # 测试输入文件
flex_output_executable="clex"     # 最终可执行文件
test_output="output.tsv"          # 测试输出文件

log(){
    echo "[$(date +"%Y-%m-%d %H:%M:%S")] : $1 "
}
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
    log "Cleaning generated files..."
    if [ -d "flex" ]; then
        cd flex || exit 1
        rm -f "$flex_output" "$flex_output_executable" "$test_output"
        log "Removed: $flex_output, $flex_output_executable, $test_output"
        cd ..
    else
        log "flex directory not found, nothing to clean."
    fi
    exit 0
}

# 正常的构建流程
build() {
    # 获取当前flex版本（如果未安装，变量为空）
    flex_version=$(flex --version 2>/dev/null | awk '{print $2}')
    gcc_version=$(gcc --version 2>/dev/null | head -n 1 | awk '{print $3}')
    
    # 检查并安装flex 2.6.4
    if [ "$flex_version" = "2.6.4" ]; then
        log "flex 2.6.4 is already installed"
    else
        log "flex $flex_version is not installed, installing flex 2.6.4..."
        apt install -y flex
    fi

    # 检查gcc版本
    if [ -n "$gcc_version" ]; then
        log "gcc version $gcc_version is installed"
    else
        log "gcc is not installed, installing gcc..."
        apt install -y build-essential
    fi

    # 进入flex目录并执行编译+运行
    if [ -d "flex" ]; then
        cd flex || exit 1

        # 1. 用flex生成C代码
        if [ -f "$flex_file" ]; then
            log "Found flex file: $flex_file"
        else
            log "错误：flex文件 $flex_file 未找到"
            exit 1
        fi
        log "生成flex代码: $flex_output"
        flex -o "$flex_output" "$flex_file"

        # 2. 用gcc编译生成可执行文件
        if [ -f "$flex_output" ]; then
            log "flex代码生成成功: $flex_output"
        else
            log "错误：flex代码 $flex_output 未生成"
            exit 1
        fi
        log "编译可执行文件: $flex_output_executable"
        gcc -o "$flex_output_executable" "$flex_output" -lfl

        # 3. 执行可执行文件，并重定向输入
        if [ -f "$flex_output_executable" ]; then
            log "执行程序，输入文件: $flex_input_file"
            ./"$flex_output_executable" < "$flex_input_file" > "$test_output"
        else
            log "错误：可执行文件 $flex_output_executable 未生成"
            exit 1
        fi

        cd ..
    else
        log "错误：flex目录不存在"
        exit 1
    fi

    log "执行完成"
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