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
    gpp_version=$(g++ --version 2>/dev/null | head -n 1 | awk '{print $3}')

    # 检查并安装flex 2.6.4
    if [ "$flex_version" = "2.6.4" ]; then
        log "flex 2.6.4 is already installed"
    else
        log "flex $flex_version is not installed, installing flex 2.6.4..."
        sudo apt install -y flex
    fi

    # 检查g++版本
    if [ -n "$gpp_version" ]; then
        log "g++ version $g++_version is installed"
    else
        log "g++ is not installed, installing g++..."
        sudo apt install -y build-essential
    fi
    
    # 检查mingw-w64
    gpp86version=$(x86_64-w64-mingw32-g++ --version 2>/dev/null | head -n 1 | awk '{print $3}')
    if [ -n "$gpp86version" ]; then
        log "mingw-w64 is installed"
    else
        log "mingw-w64 is not installed, installing mingw-w64..."
        sudo apt install -y mingw-w64
    fi

    # 检查cmake
    cmakeversin=$(cmake --version | head -n 1 | awk '{print $3}')
    if [ -n "$cmakeversin" ]; then
        log "cmake is installed"
    else
        log "cmake is not installed, installing cmake..."
        sudo apt install -y cmake
    fi
    
    # 检查make
    makeversion=$(make --version | head -n 1 | awk '{print $3}')
    if [ -n "$makeversion" ]; then
        log "make is installed"
    else
        log "make is not installed, installing make..."
        sudo apt install -y make
    fi
    # 检查 vcpkg
    vcpkgversin=$(vcpkg version | awk '{print $2}')
    if [ -n "$vcpkgversin" ]; then
        log "vcpkg is installed"
    else
        log "vcpkg is not installed, installing vcpkg..."
        path=$PWD
        git clone https://github.com/microsoft/vcpkg.git ~/vcpkg --depth 1
        cd ~/vcpkg

        chmod +x bootstrap-vcpkg.sh
        ./bootstrap-vcpkg.sh

        # 配置环境变量（修复路径拼接问题）
        echo "export VCPKG_ROOT=$HOME/vcpkg" >> ~/.bashrc
        echo "export PATH=\$VCPKG_ROOT:\$PATH" >> ~/.bashrc

        export VCPKG_ROOT="$HOME/vcpkg"
        export PATH="$VCPKG_ROOT:$PATH"

        # 让当前 shell 立即生效
        source ~/.bashrc

        cd "$path"
        log "vcpkg 安装完成"
    fi
    
    # 检查并安装 Linux 版本的 glfw3
    vcpkg list | grep "glfw3:x64-linux" >/dev/null
    if [ $? -eq 0 ]; then
        log "glfw3:x64-linux 已安装"
    else
        log "glfw3:x64-linux 未安装"
        sudo apt-get install -y curl build-essential mesa-common-dev libxinerama-dev \
        libglu1-mesa-dev libxcursor-dev libxrandr-dev libxi-dev libxxf86vm-dev
        vcpkg install glfw3:x64-linux
    fi
    
    # 检查并安装 Windows mingw 版本的 glfw3（用于交叉编译）
    vcpkg list | grep "glfw3:x64-mingw-dynamic" >/dev/null
    if [ $? -eq 0 ]; then
        log "glfw3:x64-mingw-dynamic 已安装"
    else
        log "glfw3:x64-mingw-dynamic 未安装，正在安装..."
        vcpkg install glfw3:x64-mingw-dynamic
        if [ $? -ne 0 ]; then
            log "警告：glfw3:x64-mingw-dynamic 安装失败，Windows 版本可能无法正常工作"
        fi
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
            cmake .. -DCMAKE_TOOLCHAIN_FILE="$MINGW_TOOLCHAIN" -DBUILD_WINDOWS=ON
            make -j4
            
            if [ $? -ne 0 ]; then
                log "错误：Windows版本编译失败"
                exit 1
            fi

            log "Windows版本编译成功: build-win/bin/${flex_output_executable}.exe"
            cd ..

            # 4. 拷贝 Windows 运行所需的 DLL 文件
            log "========== 拷贝 Windows 运行时依赖 =========="
            
            # 检查 VCPKG_ROOT 环境变量
            if [ -z "$VCPKG_ROOT" ]; then
                log "警告: VCPKG_ROOT 环境变量未设置，尝试使用默认路径 ~/vcpkg"
                VCPKG_ROOT="$HOME/vcpkg"
            fi
            
            # GLFW3 DLL 路径
            GLFW_DLL_SRC="$VCPKG_ROOT/installed/x64-mingw-dynamic/bin/glfw3.dll"
            GLFW_DLL_DST="build-win/bin/glfw3.dll"
            
            if [ -f "$GLFW_DLL_SRC" ]; then
                cp "$GLFW_DLL_SRC" "$GLFW_DLL_DST"
                log "已拷贝 glfw3.dll -> build-win/bin/"
            else
                log "警告: 未找到 glfw3.dll ($GLFW_DLL_SRC)"
                log "提示: 请确保已安装 glfw3:x64-mingw-dynamic"
            fi
            
            # 如果需要其他 DLL（如 OpenGL 相关），也可以在这里添加
            # 例如：libwinpthread-1.dll, libstdc++-6.dll, libgcc_s_seh-1.dll 等
            # 这些是 MinGW 运行时库，如果静态链接则不需要
            
            log "Windows 运行时依赖拷贝完成"
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