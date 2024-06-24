cmake_minimum_required(VERSION 3.20)
# 项目名称
project(ZsEngine)
# C++ 标准 : 20
set(CMAKE_CXX_STANDARD 20)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -std=c++20  \
    -I /usr/local/include/c++/13.2.0    \
    -I /usr/local/include/c++/13.2.0/x86_64-pc-linux-gnu    \
    -L /usr/local/lib64 \
    -L /usr/local/lib32 \
    ")
endif ()
# 构建方式 默认Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()
# 输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/build/x64/Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/build/x64/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/build/x64/Debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/build/x64/Release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/build/x64/Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/build/x64/Release")
# 包含子目录
add_subdirectory(Core)
add_subdirectory(Editor)
add_subdirectory(Resource)
add_subdirectory(Scene)
add_subdirectory(Test)
add_subdirectory(ThirdPartyManager)
