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
include_directories(${PROJECT_SOURCE_DIR}/include/imgui)
include_directories(${PROJECT_SOURCE_DIR}/include/CLI)

# ============================================================
#  核心源文件（手动指定，避免引入不需要的平台后端）
# ============================================================

# ImGui 核心文件
set(IMGUI_SOURCES
    ${PROJECT_SOURCE_DIR}/src/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/src/imgui/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/src/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/src/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/src/imgui/imgui_widgets.cpp
)

# 项目源文件
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/TokenStream.cpp
    ${PROJECT_SOURCE_DIR}/src/Type.cpp
    ${PROJECT_SOURCE_DIR}/src/lex.yy.cpp  # flex 生成的文件（需要先运行 flex）
)

# GUI 相关文件（默认不包含）
set(GUI_SOURCES "")
set(ENABLE_GUI FALSE)

# ============================================================
#  根据构建类型添加后端文件
# ============================================================

if(BUILD_WINDOWS OR CMAKE_CROSSCOMPILING)
    set(ENABLE_GUI TRUE)
    message(STATUS "WINDOWS_BUILD mode: GUI support enabled")
    
    # GLFW + OpenGL3 后端
    set(GUI_SOURCES
        ${PROJECT_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/backends/imgui_impl_opengl3.cpp
        ${PROJECT_SOURCE_DIR}/include/imgui/misc/cpp/imgui_stdlib.cpp
    )
    
    # 添加 GUI 源文件
    list(APPEND PROJECT_SOURCES
        ${PROJECT_SOURCE_DIR}/src/gui.cpp
    )
else()
    # Linux 命令行模式：不添加 GUI 和后端文件
    message(STATUS "Linux build: GUI and backend not included")
endif()

# 合并所有源文件
set(PARSER_SOURCES
    ${IMGUI_SOURCES}
    ${PROJECT_SOURCES}
    ${GUI_SOURCES}
)

# 生成主程序（必须先创建 target，才能设置 target 属性）
add_executable(parser ${PARSER_SOURCES})

# ============================================================
#  编译器警告和链接库（必须在 add_executable 之后）
# ============================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 基础警告
    target_compile_options(parser PRIVATE -Wall -Wextra)

    # 忽略 flex 生成的代码中的未使用函数警告
    target_compile_options(parser PRIVATE -Wno-unused-function)

    # Windows 交叉编译配置
    if(CMAKE_CROSSCOMPILING)
        # 静态链接，避免缺少 DLL
        target_link_options(parser PRIVATE -static)
        
        # 定义 WINDOWS_BUILD 宏
        target_compile_definitions(parser PRIVATE WINDOWS_BUILD)
        
        # 设置 vcpkg 路径（交叉编译时使用 mingw 版本的 glfw3）
        set(VCPKG_MINGW_PATHS "")
        if(DEFINED ENV{VCPKG_ROOT})
            list(APPEND VCPKG_MINGW_PATHS
                "$ENV{VCPKG_ROOT}/installed/x64-mingw-dynamic"
                "$ENV{VCPKG_ROOT}/installed/x64-mingw-static"
            )
            set(CMAKE_PREFIX_PATH "${VCPKG_MINGW_PATHS}")
            message(STATUS "Using vcpkg mingw paths: $ENV{VCPKG_ROOT}")
        endif()
        
        # 查找 glfw3
        find_package(glfw3 QUIET)
        if(glfw3_FOUND)
            target_link_libraries(parser glfw)
            message(STATUS "Found glfw3 via vcpkg")
        else()
            message(WARNING "glfw3 not found for Windows target. GUI will not work properly.")
            message(STATUS "Please install: vcpkg install glfw3:x64-mingw-dynamic")
            # 即使找不到 glfw3，也继续编译（只是 GUI 无法正常工作）
        endif()
        
        # Windows 特定链接库
        target_link_libraries(parser opengl32 gdi32)
    endif()
endif()

# 输出配置信息
message(STATUS "========================================")
message(STATUS "Parser Build Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Cross Compiling: ${CMAKE_CROSSCOMPILING}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "GUI Support: ${ENABLE_GUI}")
message(STATUS "Source Files:")
foreach(src ${PARSER_SOURCES})
    message(STATUS "  - ${src}")
endforeach()
message(STATUS "========================================")

