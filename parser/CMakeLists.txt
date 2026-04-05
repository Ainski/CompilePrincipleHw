# ============================================================
#  CMakeLists.txt for Parser (Recursive Descent Parser)
#  编译原理大作业 - 语法分析器
#  Linux 下一键编译：Linux 可执行文件 + Windows .exe
# ============================================================
cmake_minimum_required(VERSION 3.10)

project(Parser VERSION 1.0 LANGUAGES CXX)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 统一输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 头文件路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 自动收集源文件
file(GLOB_RECURSE PARSER_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 生成主程序（Linux/Windows 版本）
add_executable(parser ${PARSER_SOURCES})

# 编译器警告和选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 基础警告
    target_compile_options(parser PRIVATE -Wall -Wextra)
    
    # 忽略 flex 生成的代码中的未使用函数警告
    target_compile_options(parser PRIVATE -Wno-unused-function)
    
    # Windows 交叉编译配置
    if(CMAKE_CROSSCOMPILING)
        # 静态链接，避免缺少 DLL
        target_link_options(parser PRIVATE -static)
        
        # Windows 特有的编译选项
        if(WIN32)
            target_compile_definitions(parser PRIVATE WINDOWS_BUILD)
        endif()
    endif()
endif()

# 输出配置信息
message(STATUS "========================================")
message(STATUS "Parser Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Cross Compiling: ${CMAKE_CROSSCOMPILING}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")