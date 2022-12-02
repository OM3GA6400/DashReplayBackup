cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTERSIONS OFF)

set(PROJECT_NAME DashReplayAPI3)
project(${PROJECT_NAME})

file(GLOB_RECURSE SOURCES 
    src/*.cpp
)

set(WIN32 ON)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(
    ${PROJECT_NAME} PRIVATE
    libs/cocos-headers/cocos2dx
    libs/cocos-headers/cocos2dx/include
    libs/cocos-headers/cocos2dx/kazmath/include
    libs/cocos-headers/cocos2dx/platform/third_party/win32/OGLES
    libs/cocos-headers/cocos2dx/platform/win32
    libs/cocos-headers/extensions
    libs/MinHook
    libs/gd.h/include
    libs/gd.h
    libs/imgui-hook
    libs/imgui-hook/imgui
    libs/imgui-savedialog
)

add_subdirectory(libs/imgui-hook)
add_subdirectory(libs/imgui-savedialog)

target_link_libraries(
    ${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/libs/cocos-headers/cocos2dx/libcocos2d.lib
    ${CMAKE_SOURCE_DIR}/libs/cocos-headers/extensions/libExtensions.lib
    ${CMAKE_SOURCE_DIR}/libs/minhook/minhook.lib
    imgui-hook
    imgui-savedialog
    opengl32
)
