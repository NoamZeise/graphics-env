
# for use by ide for code completion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# for loading shared libs in
# executable's current folder when running on linux.
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Add compile defs to avoid including libs
# when certain cmake vars are defined.

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
