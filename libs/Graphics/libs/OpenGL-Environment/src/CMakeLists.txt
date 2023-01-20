file(GLOB OGL_RENDER_SOURCES
	  *
	  resources/*
	  parts/*
	  )

set(OpenGL-Render-Lib "OpenGL-Environment")

add_library(${OpenGL-Render-Lib} ${OGL_RENDER_SOURCES})
target_include_directories(${OpenGL-Render-Lib} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/
)

add_dependencies(${OpenGL-Render-Lib} ResourceLoader OutfacingGraphics)
target_link_libraries(${OpenGL-Render-Lib} PUBLIC ResourceLoader)
target_link_libraries(${OpenGL-Render-Lib} PUBLIC OutfacingGraphics)

if(UNIX)
    target_include_directories(${OpenGL-Render-Lib} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${OpenGL-Render-Lib} PRIVATE
      glfw
      ${FREETYPE_LIBRARIES}
      ${CMAKE_DL_LIBS})
else()
    target_include_directories(${OpenGL-Render-Lib} PUBLIC "D:/noam4/Libraries/VS/include")
    target_link_libraries(${OpenGL-Render-Lib} PRIVATE
      ${Lib}glfw3.lib
      ${Lib}freetype.lib
      bufferoverflowU.lib)
endif()			
