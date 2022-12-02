cmake_minimum_required(VERSION 3.0)

add_library(imgui-hook STATIC
    imgui_hook.cpp
    imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
    imgui/imgui_impl_opengl2.cpp
    imgui/imgui_impl_win32.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
	kiero/kiero.cpp
)
set_property(TARGET imgui-hook PROPERTY CXX_STANDARD 17)
target_include_directories(imgui-hook PRIVATE imgui
    ../libs/minhook/
)