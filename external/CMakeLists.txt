# Settings for external libs

if(NOT NO_AUDIO)
  add_subdirectory(audio)
endif()

add_subdirectory(volk)

# glfw setup
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
if(GRAPHICS_STATIC)
 if(MSVC)
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")
  endif()
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
else()
  set(BUILD_SHARED_LIBS ON CACHE BOOL "")
endif()
add_subdirectory(glfw)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "") # dont want shared lib version of glm and volk
add_subdirectory(glm)

# assimp setup
if(NOT NO_ASSIMP)
  if(NOT GRAPHICS_STATIC)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "")
  else()
    # need this for static build working
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
    set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "")
  endif()
  set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "")
  set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
  set(ASSIMP_INSTALL OFF CACHE BOOL "")
  set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
  set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
  # by default only build a few imports
  option(ASSIMP_BUILD_FBX_IMPORTER "Build importer for fbx models" ON)
  option(ASSIMP_BUILD_OBJ_IMPORTER "Build importer for obj models" ON)
  option(ASSIMP_BUILD_GLTF_IMPORTER "Build importer for gltf models" ON)
  add_subdirectory(assimp)
endif()

if(GRAPHICS_STATIC)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
else()
  set(BUILD_SHARED_LIBS ON CACHE BOOL "")
endif()

if(NOT NO_FREETYPE)
  # disable freetype optional dependancies
  set(FT_DISABLE_ZLIB ON CACHE BOOL "")
  set(FT_DISABLE_BZIP2 ON CACHE BOOL "")
  set(FT_DISABLE_PNG ON CACHE BOOL "")
  set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "")
  set(FT_DISABLE_BROTLI ON CACHE BOOL "")
  add_subdirectory(freetype)
endif()
