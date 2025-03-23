#设置 CMake 的最低版本要求为 3.0
cmake_minimum_required(VERSION 3.0)
project(MyMuduoCore) #锁定项目的工作目录

#设置全局的c++标准
set (CMAKE_CXX_STANDARD 11) 
set (CMAKE_CXX_STANDARD_REQUIRED True)

#设置生成的 库文件（动态库 .so 或静态库 .a）输出到 ${PROJECT_SOURCE_DIR}/lib 目录。
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#设置静态库输出路径
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib) # 设置静��库输出路径
#设置全局链接库
set(LIBS
    pthread
)

# 添加子目录，并自动获取其中的CMakeLists.txt
add_subdirectory(src)
add_subdirectory(example)