cmake_minimum_required(VERSION 3.0)

add_library(imgui-savedialog STATIC
	ImGuiFileDialog.cpp
)
set_property(TARGET imgui-savedialog PROPERTY CXX_STANDARD 17)
target_include_directories(imgui-savedialog PRIVATE imgui
    ../imgui-hook/imgui/
)