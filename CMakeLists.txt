cmake_minimum_required(VERSION 3.10)


#customize
if(GFX_ENV_VULKAN)
  set(ProjectName "Vulkan-Environment")
  add_definitions(-DGFX_ENV_VULKAN)
elseif(GFX_ENV_OPENGL)
  set(ProjectName "OpenGL-Environment")
  add_definitions(-DGFX_ENV_OPENGL)
endif()
#windows only
set(Lib "D:/noam4/Libraries/VS/lib/x64/")
set(Include "D:/noam4/Libraries/VS/include")

project(${ProjectName} VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

#packages
if (GFX_ENV_VULKAN)
  find_package(Vulkan REQUIRED)
endif()

if (UNIX)
    if (GFX_ENV_VULKAN)
      add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
    endif()
    find_package(glfw3 3.3 REQUIRED)
    find_package(Freetype REQUIRED)
endif()

#add executables
file(GLOB SOURCES src/*)
if(GFX_ENV_VULKAN)
  file(GLOB GRAPHICS-RENDER src/vulkan-render/*)
  add_executable(${ProjectName} ${SOURCES}
                              ${GRAPHICS-RENDER}
                              ${PROJ_INCLUDE})
elseif(GFX_ENV_OPENGL)
  file(GLOB OGL_RENDER src/opengl-render/*)
  file(GLOB OGL_RENDER_RESOURCES src/opengl-render/resources/*)
  add_executable(${ProjectName}   ${SOURCES}
                                ${OGL_RENDER}
                                ${OGL_RENDER_RESOURCES}
                                ${PROJ_INCLUDE})
endif()
#link
target_include_directories(${ProjectName} PUBLIC include/)
if(UNIX)
    target_include_directories(${ProjectName} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    if(GFX_ENV_VULKAN)
    target_link_libraries(${ProjectName} glfw
                                         Vulkan::Vulkan
                                         ${FREETYPE_LIBRARIES}
                                         assimp
                                         sndfile
                                         portaudio)
    elseif(GFX_ENV_OPENGL)
      target_link_libraries(${ProjectName} glfw
                                     assimp
                                     ${FREETYPE_LIBRARIES}
                                     sndfile
                                     portaudio
                                     ${CMAKE_DL_LIBS})
    endif()
else()
    target_include_directories(${ProjectName} PUBLIC ${Include})
    if(GFX_ENV_VULKAN)
    target_link_libraries(${ProjectName} ${Lib}glfw3.lib
                                         ${Lib}freetype.lib
                                         ${Lib}vulkan-1.lib
                                         ${Lib}assimp-vc140-mt.lib
                                         ${Lib}portaudio_x64.lib
                                         ${Lib}sndfile.lib
                                        bufferoverflowU.lib)
    elseif(GFX_ENV_OPENGL)
      target_link_libraries(${ProjectName} ${Lib}glfw3.lib
                                         ${Lib}freetype.lib
                                         ${Lib}assimp-vc140-mt.lib
                                         ${Lib}portaudio_x64.lib
                                         ${Lib}sndfile.lib)
    endif()
endif()
