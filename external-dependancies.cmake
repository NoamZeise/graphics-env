# external dependancy versions

cmake_policy(SET CMP0077 NEW) # override external lib options with vars
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW) # fetchcontent timestamps are extract time
endif()

include(FetchContent)

set(graphics_assimp_url
  https://github.com/assimp/assimp/archive/refs/tags/v5.4.3.zip)

set(graphics_audio_commit
  449347dfc68dd7e7dab16343061b9707ff06e0f5)

set(graphics_freetype_url
  https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.xz)

set(graphics_glfw_commit
  7b6aead9fb88b3623e3b3725ebb42670cbe4c579) # v3.4

set(graphics_glm_commit
  0af55ccecd98d4e5a8d1fad7de25ba429d60e863) # v1.0.1

set(graphics_spirv_cross_commit
  2ccc81fd826e4dd4a2db2f94b8e6eb738a89f5f1) # 29 Apr 2024

set(graphics_volk_commit
  0b17a763ba5643e32da1b2152f8140461b3b7345) # v1.4.304


# Settings for external libs

if(GRAPHICS_STATIC)
 if(MSVC)
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "")
  endif()
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
else()
  set(BUILD_SHARED_LIBS ON CACHE BOOL "")
endif()

# glfw setup
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG ${graphics_glfw_commit}
  FIND_PACKAGE_ARGS NAMES glfw3
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
FetchContent_MakeAvailable(glfw)

# assimp setup
if(NOT NO_ASSIMP)
  if(GRAPHICS_STATIC)
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
  FetchContent_Declare(
    assimp
    URL ${graphics_assimp_url}
    FIND_PACKAGE_ARGS
  )
  FetchContent_MakeAvailable(assimp)
endif()

# freetype setup
if(NOT NO_FREETYPE)
  FetchContent_Declare(freetype
    URL ${graphics_freetype_url}
    FIND_PACKAGE_ARGS NAMES Freetype
  )
  # disable freetype optional dependancies
  set(FT_DISABLE_ZLIB ON CACHE BOOL "")
  set(FT_DISABLE_BZIP2 ON CACHE BOOL "")
  set(FT_DISABLE_PNG ON CACHE BOOL "")
  set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "")
  set(FT_DISABLE_BROTLI ON CACHE BOOL "")
  FetchContent_MakeAvailable(freetype)
endif()

# spirv-cross setup 
# static only works for me with gcc?
FetchContent_Declare(spirv_cross
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
  GIT_TAG ${graphics_spirv_cross_commit}  
)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "")
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "")
set(SPIRV_CROSS_CLI OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "")
FetchContent_MakeAvailable(spirv_cross)

# dont want shared lib version of glm, volk, audio
set(BUILD_SHARED_LIBS OFF)

FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG ${graphics_glm_commit}
  FIND_PACKAGE_ARGS
)

FetchContent_Declare(volk
  GIT_REPOSITORY https://github.com/zeux/volk.git
  GIT_TAG ${graphics_volk_commit}
)

FetchContent_MakeAvailable(glm volk)

if(NOT NO_AUDIO)
  FetchContent_Declare(audio
    GIT_REPOSITORY https://github.com/NoamZeise/audio.git
    GIT_TAG ${graphics_audio_commit}
  )
  FetchContent_MakeAvailable(audio)
endif()
