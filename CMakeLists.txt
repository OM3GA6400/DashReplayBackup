cmake_minimum_required(VERSION 3.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(PROJECT_NAME DashReplayAPI3)
project(${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
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
)

add_subdirectory(libs/minhook)
add_subdirectory(libs/imgui-hook)

target_link_libraries(${PROJECT_NAME} minhook)
target_link_libraries(${PROJECT_NAME} imgui-hook)
target_link_libraries(${PROJECT_NAME} opengl32)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/cocos-headers/cocos2dx/libcocos2d.lib)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libs/cocos-headers/extensions/libExtensions.lib)