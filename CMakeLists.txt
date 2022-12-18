cmake_minimum_required(VERSION 3.10)

#customize
set(ProjectName "Graphics-Environment")
#windows only
set(Lib "D:/noam4/Libraries/VS/lib/x64/")
set(Include "D:/noam4/Libraries/VS/include")

project(${ProjectName} VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wl,--no-as-needed -ldl")
    add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
    find_package(glfw3 3.3 REQUIRED)
    find_package(Freetype REQUIRED)
endif()

#add executables
file(GLOB SOURCES src/*
  src/render/*
  src/render/render-shared/*
  src/render/render-shared/resources/*
  src/render/render-shared/resources/model/*
  )
list(FILTER SOURCES EXCLUDE REGEX ".*#+")
file(GLOB VK_RENDER src/render/vulkan-render/*
  src/render/vulkan-render/resources/*
  src/render/vulkan-render/parts/*)
list(FILTER VK_RENDER EXCLUDE REGEX ".*#+")


file(GLOB OGL_RENDER src/render/opengl-render/*
  src/render/opengl-render/resources/*)
list(FILTER OGL_RENDER EXCLUDE REGEX ".*#+")

add_executable(${ProjectName}   ${SOURCES}
  ${OGL_RENDER}
  ${VK_RENDER}
  ${PROJ_INCLUDE})

#link
target_include_directories(${ProjectName} PUBLIC include/)
if(UNIX)
    target_include_directories(${ProjectName} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${ProjectName} glfw
                                         ${FREETYPE_LIBRARIES}
                                         assimp
                                         sndfile
                                         portaudio
                                         ${CMAKE_DL_LIBS})
else()
    target_include_directories(${ProjectName} PUBLIC ${Include})
    target_link_libraries(${ProjectName} ${Lib}glfw3.lib
                                         ${Lib}freetype.lib
                                         ${Lib}vulkan-1.lib
                                         ${Lib}assimp-vc140-mt.lib
                                         ${Lib}portaudio_x64.lib
                                         ${Lib}sndfile.lib
                                        bufferoverflowU.lib)
endif()
