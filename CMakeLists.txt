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
  

add_subdirectory(libs)

#add executables
file(GLOB SOURCES src/*)
list(FILTER SOURCES EXCLUDE REGEX ".*#+")

add_executable(${ProjectName} ${SOURCES}  ${OGL_RENDER}  ${PROJ_INCLUDE})
  
#linke
target_include_directories(${ProjectName} PUBLIC
					  include/
					  )

target_link_libraries(${ProjectName} Vulkan-Renderer)
target_link_libraries(${ProjectName} OpenGL-Renderer)
if(UNIX)
    target_include_directories(${ProjectName} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${ProjectName} glfw
                                         ${CMAKE_DL_LIBS})
else()
    target_include_directories(${ProjectName} PUBLIC ${Include})
    target_link_libraries(${ProjectName} ${Lib}glfw3.lib)
endif()
