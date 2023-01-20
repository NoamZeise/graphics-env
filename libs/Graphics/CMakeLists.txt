cmake_minimum_required(VERSION 3.10)

project(Graphics)

find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

add_subdirectory(libs)

add_subdirectory(src)
