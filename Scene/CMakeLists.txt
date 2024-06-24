project(Scene)
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
include_directories(${PARENT_DIR}/Core)
include_directories(${PARENT_DIR}/Resource)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 库
set(LIBS
        Core
        Resource
)

# 需要编译的源码
file(GLOB_RECURSE SCENE_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
)
# 添加宏定义
add_compile_definitions(SCENE_EXPORTS)
# 生成DLL
add_library(${PROJECT_NAME} SHARED ${SCENE_SOURCE})
target_link_libraries(${PROJECT_NAME} ${LIBS})