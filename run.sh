# !/bin/bash
# build environment
# please run this shell with sudo

# 定义文件路径变量（不依赖外部命令）
flex_file="lexer.l"               # flex源文件
flex_output="lex.yy.cpp"           # flex生成的C文件
flex_input_file="input.rs"        # 测试输入文件
flex_output_executable="parser"     # 最终可执行文件
test_output="output.tsv"          # 词法分析输出文件
parser_src="../parser/parser.cpp" # 语法分析器源文件
parser_bin="../parser/parser"     # 语法分析器可执行文件
parse_output="parse_tree.txt"     # 语法分析输出文件

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
        cd parser/src || exit 1
        rm -f "$flex_output" "$flex_output_executable" "$test_output"
        log "Removed: $flex_output, $flex_output_executable, $test_output"
        cd ../..
    else
        log "flex directory not found, nothing to clean."
    fi
    exit 0
}

# 正常的构建流程
build() {
    # 获取当前flex版本（如果未安装，变量为空）
    flex_version=$(flex --version 2>/dev/null | awk '{print $2}')
    g++_version=$(g++ --version 2>/dev/null | head -n 1 | awk '{print $3}')
    
    # 检查并安装flex 2.6.4
    if [ "$flex_version" = "2.6.4" ]; then
        log "flex 2.6.4 is already installed"
    else
        log "flex $flex_version is not installed, installing flex 2.6.4..."
        sudo apt install -y flex
    fi

    # 检查g++版本
    if [ -n "$g++_version" ]; then
        log "g++ version $g++_version is installed"
    else
        log "g++ is not installed, installing g++..."
        sudo apt install -y build-essential
    fi

    # 检查cmake
    if command -v cmake &> /dev/null; then
        log "cmake is installed"
    else
        log "cmake is not installed, installing cmake..."
        sudo apt install -y cmake
    fi
    if command -v make &> /dev/null; then
        log "make is installed"
    else
        log "make is not installed, installing make..."
        sudo apt install -y make
    fi

    # 进入flex目录并执行编译+运行
    if [ -d "parser/src" ]; then
        cd parser/src || exit 1

        # 1. 用flex生成C代码
        if [ -f "$flex_file" ]; then
            log "Found flex file: $flex_file"
        else
            log "错误：flex文件 $flex_file 未找到"
            exit 1
        fi
        log "生成flex代码: $flex_output"
        flex -o "$flex_output" "$flex_file"

        # # 2. 用g++编译生成可执行文件
        # if [ -f "$flex_output" ]; then
        #     log "flex代码生成成功: $flex_output"
        # else
        #     log "错误：flex代码 $flex_output 未生成"
        #     exit 1
        # fi
        # log "编译可执行文件: $flex_output_executable"
        # g++ -o "$flex_output_executable" "$flex_output" -lfl

        # # 3. 执行可执行文件，并重定向输入
        # if [ -f "$flex_output_executable" ]; then
        #     log "执行词法分析，输入文件: $flex_input_file"
        #     ./"$flex_output_executable"
        # else
        #     log "错误：可执行文件 $flex_output_executable 未生成"
        #     exit 1
        # fi

        if [ -f "$flex_output" ]; then
            log "编译flex代码: $flex_output_executable"

            cd ..
            cmake .
            make
            if [ $? -ne 0 ]; then
                log "错误：flex代码编译失败"
                exit 1
            fi
        else
            log "错误：flex代码 $flex_output 未生成"
            exit 1
        fi

        if [ -f "bin/$flex_output_executable" ]; then
            log "执行词法分析，输入文件: $flex_input_file"
            ./"bin/$flex_output_executable"
            if [ $? -ne 0 ]; then
                log "错误：词法分析执行失败"
                exit 1
            fi
        else
            log "错误：可执行文件 $flex_output_executable 未生成"
            exit 1
        fi

        cd ..

    # 4. 编译语法分析器
    if [ -f "parser/parser.cpp" ]; then
        log "编译语法分析器: $parser_bin"
        g++ -std=c++17 -o "$parser_bin" "parser/parser.cpp"
        if [ $? -ne 0 ]; then
            log "错误：语法分析器编译失败"
            exit 1
        fi
    else
        log "警告：未找到 parser/parser.cpp，跳过语法分析"
        exit 0
    fi

    # 5. 运行语法分析器
    if [ -f "$parser_bin" ]; then
        log "运行语法分析器，输入: flex/$test_output"
        "$parser_bin" "flex/$test_output" > "flex/$parse_output"
        if [ $? -eq 0 ]; then
            log "语法分析成功，输出: flex/$parse_output"
        else
            log "错误：语法分析失败"
            cat "flex/$parse_output" >&2
            exit 1
        fi
    fi
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