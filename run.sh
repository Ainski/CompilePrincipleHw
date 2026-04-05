# !/bin/bash
# build environment
# please run this shell with sudo

# 定义文件路径变量
flex_file="lexer.l"               # flex源文件
flex_output="lex.yy.cpp"           # flex生成的C文件
flex_input_file="./testfiles/input.rs"        # 测试输入文件
flex_output_executable="parser"     # 最终可执行文件
lex_output="./testfiles/output.tsv"          # 词法分析输出文件
parse_output="./testfiles/parse_tree.txt"     # 语法分析输出文件

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
    if [ -d "parser" ]; then
        rm -f "parser/src/$flex_output"
        rm -rf "parser/build"
        rm -rf "parser/build-win"
        rm -f "$lex_output" "$parse_output"
        log "Removed generated files"
    else
        log "parser directory not found, nothing to clean."
    fi
    exit 0
}

# 正常的构建流程
build() {
    # 检查并创建 mingw-w64 工具链文件
    MINGW_TOOLCHAIN="/usr/share/cmake/mingw-w64/toolchain-x86_64-w64-mingw32.cmake"
    if [ ! -f "$MINGW_TOOLCHAIN" ]; then
        log "未找到 mingw-w64 工具链文件，正在创建..."
        sudo mkdir -p /usr/share/cmake/mingw-w64
        sudo tee "$MINGW_TOOLCHAIN" > /dev/null << 'TOOLCHAIN_EOF'
# MingW-w64 工具链文件
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
TOOLCHAIN_EOF

        if [ -f "$MINGW_TOOLCHAIN" ]; then
            log "工具链文件创建成功: $MINGW_TOOLCHAIN"
        else
            log "错误：工具链文件创建失败"
            exit 1
        fi
    else
        log "mingw-w64 工具链文件已存在"
    fi

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
    
    # 检查mingw-w64
    if command -v x86_64-w64-mingw32-g++ &> /dev/null; then
        log "mingw-w64 is installed"
    else
        log "mingw-w64 is not installed, installing mingw-w64..."
        sudo apt install -y mingw-w64
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

    # 进入parser目录并执行编译+运行
    if [ -d "parser" ]; then
        cd parser || exit 1

        # 1. 用flex生成C代码
        if [ -f "src/$flex_file" ]; then
            log "Found flex file: src/$flex_file"
        else
            log "错误：flex文件 src/$flex_file 未找到"
            exit 1
        fi
        
        log "生成flex代码: src/$flex_output"
        flex -o "src/$flex_output" "src/$flex_file"

        if [ -f "src/$flex_output" ]; then
            log "flex代码生成成功: src/$flex_output"
        else
            log "错误：flex代码 src/$flex_output 未生成"
            exit 1
        fi

        # 2. 编译 Linux 版本
        log "========== 编译 Linux 版本 =========="
        if [ -f "CMakeLists.txt" ]; then
            log "Found CMakeLists.txt, using CMake for build"
        else
            log "错误：CMakeLists.txt 未找到，无法使用CMake构建"
            exit 1
        fi
        
        mkdir -p build && cd build
        cmake ..
        make -j4
        
        if [ $? -ne 0 ]; then
            log "错误：Linux版本编译失败"
            exit 1
        fi
        
        log "Linux版本编译成功: build/bin/$flex_output_executable"
        cd ..

        # 3. 编译 Windows .exe 版本
        log "========== 编译 Windows .exe 版本 =========="
        
        # 查找 mingw-w64 的工具链文件
        MINGW_TOOLCHAIN=""
        for toolchain in \
            /usr/share/mingw/toolchain-x86_64-w64-mingw32.cmake \
            /usr/share/mingw-w64/toolchain-x86_64-w64-mingw32.cmake \
            /usr/share/cmake/mingw-w64/toolchain-x86_64-w64-mingw32.cmake \
            /usr/local/share/cmake/mingw-w64/toolchain-x86_64-w64-mingw32.cmake; do
            if [ -f "$toolchain" ]; then
                MINGW_TOOLCHAIN="$toolchain"
                break
            fi
        done
        
        if [ -n "$MINGW_TOOLCHAIN" ]; then
            log "找到 mingw-w64 工具链: $MINGW_TOOLCHAIN"
            mkdir -p build-win && cd build-win
            cmake .. -DCMAKE_TOOLCHAIN_FILE="$MINGW_TOOLCHAIN"
            make -j4
            
            if [ $? -ne 0 ]; then
                log "错误：Windows版本编译失败"
                exit 1
            fi
            
            log "Windows版本编译成功: build-win/bin/${flex_output_executable}.exe"
            cd ..
        else
            log "警告：未找到 mingw-w64 工具链文件，跳过 Windows 版本编译"
            log "提示：可以手动安装 mingw-w64 或指定正确的工具链路径"
        fi

        # 4. 执行语法分析（使用 Linux 版本）
        if [ -f "build/bin/$flex_output_executable" ]; then
            log "========== 执行语法分析 =========="
            log "输入文件: $flex_input_file"
            ./build/bin/$flex_output_executable\
            --input "$flex_input_file"\
            --lexer-output "$lex_output"\
            --parser-output "$parse_output"
            if [ $? -ne 0 ]; then
                log "错误：语法分析执行失败"
                exit 1
            fi
        else
            log "错误：可执行文件 build/bin/$flex_output_executable 未生成"
            exit 1
        fi
    else
        log "错误：parser 目录不存在"
        exit 1
    fi
    
    cd ..
    log "========== 执行完成 =========="
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