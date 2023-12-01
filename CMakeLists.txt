cmake_minimum_required(VERSION 3.14)

include("cmake/project-setup.cmake")

set(ProjectName "Graphics-Environment")

if(NO_VULKAN AND NO_OPENGL)
  message(FATAL_ERROR "Definition conflicts: all rendering backends were disabled!")
endif()

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

if(GRAPHICS_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
