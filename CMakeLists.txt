cmake_minimum_required(VERSION 3.17)
project(3D_CurrentLineVisualization)

set(CMAKE_CXX_STANDARD 17)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(3D_CurrentLineVisualization
        main.cpp
        src/services/streamline.cpp
        src/services/vector3.cpp)

target_include_directories(3D_CurrentLineVisualization PUBLIC ./headers)
target_link_libraries(3D_CurrentLineVisualization ${VTK_LIBRARIES})