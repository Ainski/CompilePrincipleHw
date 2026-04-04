# ============================================================
#  CMakeLists.txt for Parser (Recursive Descent Parser)
#  编译原理大作业 - 语法分析器
# ============================================================
cmake_minimum_required(VERSION 3.10)

project(Parser VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 输出目录设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)

# 自动收集 src 目录下的所有源文件
file(GLOB_RECURSE PARSER_SOURCES "src/*.cpp" "src/*.cc" "src/*.cxx")

# 创建语法分析器可执行文件
add_executable(parser ${PARSER_SOURCES})

# 编译器警告选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(parser PRIVATE -Wall -Wextra)
endif()
