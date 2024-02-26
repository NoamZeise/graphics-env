cmake_minimum_required(VERSION 3.21)

include("cmake/project-setup.cmake")

set(ProjectName "Graphics-Environment")

set(GRAPHICS_AS_ROOT OFF)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(GRAPHICS_AS_ROOT ON)
endif()
option(GRAPHICS_BUILD_STATIC "build libraries statically" OFF)
option(GRAPHICS_BUILD_EXAMPLES "Build example executables" ${GRAPHICS_AS_ROOT})
option(NO_VULKAN "Don't enable the Vulkan Backend" OFF)
option(NO_OPENGL "Don't enable the OpenGL Backend" OFF)

if(NO_VULKAN AND NO_OPENGL)
  message(FATAL_ERROR "Definition conflicts: all rendering backends were disabled!")
endif()

if(GRAPHICS_BUILD_STATIC)
  set(VKENV_BUILD_STATIC ON)
  set(BUILD_SHARED_LIBS OFF)
  if(MSVC)
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
endif()

project(Graphics-Env-Proj VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(libs)
add_subdirectory(src)

if(GRAPHICS_BUILD_EXAMPLES OR PROJECT_IS_TOP_LEVEL)
  add_subdirectory(examples)
endif()
