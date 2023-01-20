cmake_minimum_required(VERSION 3.10)

set(OpenGL-Environment-Proj "OpenGL-Environment")

project(${OpenGL-Environment-Proj} VERSION 1.0)

if(UNIX)
    find_package(glfw3 3.3 REQUIRED)
    find_package(Freetype REQUIRED)
endif()

add_subdirectory(src)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
