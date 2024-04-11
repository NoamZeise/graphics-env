cmake_minimum_required(VERSION 3.21)

include("cmake/project-setup.cmake")

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(GRAPHICS_AS_ROOT ON)
endif()
# Project options
option(GRAPHICS_STATIC "Vuild libraries statically" OFF)
option(GRAPHICS_EXAMPLES "Build example executables" ${GRAPHICS_AS_ROOT})
option(NO_VULKAN "Don't enable the Vulkan Renderer" OFF)
option(NO_OPENGL "Don't enable the OpenGL Renderer" OFF)
option(NO_AUDIO "Don't use audio libs" OFF)
option(NO_ASSIMP "Don't use assimp - no 3d model loading" OFF)
option(NO_FREETYPE "Don't use freetype - no font file loading" OFF)
if(NO_VULKAN AND NO_OPENGL)
  message(FATAL_ERROR "Definition conflicts: all rendering backends were disabled!")
endif()

# static build settings
if(GRAPHICS_STATIC)
  set(BUILD_SHARED_LIBS OFF)
  if(MSVC)
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
endif()

project(graphics-env-project VERSION 1.0)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(external)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(libs)
add_subdirectory(src)

if(GRAPHICS_EXAMPLES)
  add_subdirectory(examples)
endif()
