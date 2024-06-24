project(Core)
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
# 库目录
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    link_directories("${PARENT_DIR}/Lib/debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    link_directories("${PARENT_DIR}/Lib/release")
else()
    message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}")
endif()
# 头目录
include_directories(${PARENT_DIR})
include_directories(${PARENT_DIR}/ThirdParty)
include_directories(${PARENT_DIR}/ThirdPartyManager)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 库
set(LIBS ThirdPartyManager)

# 需要编译的源码
file(GLOB_RECURSE CORE_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
)
# 添加宏定义
add_compile_definitions(CORE_EXPORTS)
# 生成DLL
add_library(${PROJECT_NAME} SHARED ${CORE_SOURCE})
target_link_libraries(${PROJECT_NAME} ${LIBS})