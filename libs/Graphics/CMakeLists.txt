cmake_minimum_required(VERSION 3.10)

project(Graphics)

if(UNIX)
  find_package(glfw3 REQUIRED)
  find_package(Freetype REQUIRED)
endif()

add_subdirectory(libs)

add_subdirectory(src)
