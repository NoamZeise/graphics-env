cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(GameHelper)

if(UNIX)
  find_package(glfw3 3.3 REQUIRED)
endif()

add_subdirectory(src)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(demo)
endif()
