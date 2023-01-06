cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(PROJECT_NAME DashReplayAPI3)
project(${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    libs/gd.h/include
    libs/gd.h
    libs/imgui-hook
    libs/imgui-hook/imgui
)

add_subdirectory(libs/minhook)
add_subdirectory(libs/imgui-hook)
add_subdirectory(libs/cocos-headers)

target_link_libraries(${PROJECT_NAME} minhook imgui-hook opengl32 cocos2d)
