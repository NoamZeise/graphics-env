file(GLOB OGL_RENDER_SOURCES
	  *
	  resources/*
	  parts/*
)


set(OpenGL-Render-Lib "OpenGL-Environment")

add_library(${OpenGL-Render-Lib} ${OGL_RENDER_SOURCES})

add_dependencies(${OpenGL-Render-Lib} resource-loader graphics-api glfw)

target_link_libraries(${OpenGL-Render-Lib} PUBLIC graphics-api glfw)

target_link_libraries(${OpenGL-Render-Lib} PRIVATE resource-loader)

if(NOT NO_FREETYPE)
  add_dependencies(${OpenGL-Render-Lib} freetype)
  target_link_libraries(${OpenGL-Render-Lib} PRIVATE freetype)
endif()

target_include_directories(${OpenGL-Render-Lib} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../glad-include/
)
