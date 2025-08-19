cmake_minimum_required(VERSION 4.0)
project(findmywindows)

set(CMAKE_CXX_STANDARD 26)

add_executable(findmywindows main.cpp
        gui.cpp
        gui.h
        tabs.cpp
        tabs.h
)

find_package(imgui CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(findmywindows PRIVATE
        glfw
        glad::glad
        imgui::imgui
)
