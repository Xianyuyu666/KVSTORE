cmake_minimum_required(VERSION 3.16) # cmake版本要求
project(kvstore CXX) # 项目名字

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成一份json文件包含源文件完整编译命令
set(CMAKE_CXX_STANDARD 17)  # 用c++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 编译器需要支持c++17

# 用户如果手动指定构建类型，默认Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic) # 更多警告

# Debug模式下开启ASAN
set(CMAKE_CXX_FLAGS_DEBUG "-fsanitize=address -fno-omit-frame-pointer -g")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address")

add_executable(kvstore src/main.cpp src/reactor.cpp src/conn.cpp)
add_executable(test src/test.cpp src/reactor.cpp src/conn.cpp)

target_include_directories(kvstore PRIVATE include)
target_include_directories(test PRIVATE include)


