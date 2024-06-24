project(Test)
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
# 库
set(LIBS
        SDL2
        SDL2main
        ThirdPartyManager
        Core
        Resource
        Scene
        Editor
)

# 需要编译的源码
file(GLOB_RECURSE TEST_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
)
add_executable(${PROJECT_NAME} ${TEST_SOURCE})
target_link_libraries(${PROJECT_NAME} ${LIBS})