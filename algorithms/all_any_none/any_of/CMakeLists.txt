cmake_minimum_required(VERSION 3.17)
project(examples VERSION 1.0.0)

find_package(HPX REQUIRED)

add_hpx_executable(exe
    ESSENTIAL
    SOURCES main.cpp
    COMPONENT_DEPENDENCIES iostreams
)

target_compile_options(exe PUBLIC "-march=native" "-fno-tree-vectorize")
target_link_libraries(exe PUBLIC HPX::hpx HPX::iostreams_component)