cmake_minimum_required(VERSION 3.10)

set(ProjectName "Graphics-Environment")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(STATIC_FLAG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

project(Graphics-Env-Proj VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
  
add_subdirectory(libs)
add_subdirectory(src)
