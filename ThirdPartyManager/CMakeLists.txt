project(ThirdPartyManager)
# 父目录
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
# 库目录
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    link_directories("${PARENT_DIR}/Lib/debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    link_directories("${PARENT_DIR}/Lib/release")
else()
    message(FATAL_ERROR "Unsupported build type: ${CMAKE_BUILD_TYPE}")
endif()
# 头文件
# ThirdParty
include_directories("${PARENT_DIR}")
include_directories("${PARENT_DIR}/ThirdParty")
# ThirdPartyManager
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/utils")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/core/common/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/core/config/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/core/info/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/assimpParty/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/imguiParty/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/jsoncppParty/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdlParty/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vkParty/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/vkParty/include/vkobject")

set(LIBS
        vulkan-1
        assimp-vc143-mt
        imgui
        jsoncpp
        jsoncpp_static
        SDL2
        shaderc_combined
        spirv-reflect
)

# 需要编译的源码
file(GLOB_RECURSE THIRDPARTYMANAGER_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
)
# 添加宏定义
add_compile_definitions(THIRDPARTYMANAGER_EXPORTS)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_compile_definitions(_DEBUG)
endif ()
# 生成DLL
add_library(${PROJECT_NAME} SHARED ${THIRDPARTYMANAGER_SOURCE})
target_link_libraries(${PROJECT_NAME} ${LIBS})
