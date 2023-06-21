cmake_minimum_required(VERSION 3.14)

set(ProjectName "Graphics-Environment")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

if(GRAPHICS_BUILD_STATIC)
  set(VKENV_BUILD_STATIC ON)
  set(BUILD_SHARED_LIBS OFF)
  if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
endif()

project(Graphics-Env-Proj VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
  
add_subdirectory(libs)
add_subdirectory(src)
