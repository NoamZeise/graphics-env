cmake_minimum_required(VERSION 3.14)

set(ProjectName "Graphics-Environment")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

if(NO_ASSIMP)
  add_compile_definitions(NO_ASSIMP)
endif()

if(NO_FREETYPE)
  add_compile_definitions(NO_FREETYPE)
endif()

if(NO_AUDIO)
  add_compile_definitions(NO_AUDIO)
endif()

if(NO_VULKAN)
  add_compile_definitions(NO_VULKAN)
endif()

if(NO_OPENGL)
  add_compile_definitions(NO_OPENGL)
endif()

if(NO_VULKAN AND NO_OPENGL)
  message(FATAL_ERROR "Definition conflicts: Can't disable all rendering backends!")
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

if(GRAPHICS_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
