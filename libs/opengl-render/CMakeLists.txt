file(GLOB OGL_RENDER_SOURCES
	  *
	  resources/*
	  parts/*
	  )

set(OpenGL-Render-Project "OpenGL-Renderer")

add_library(${OpenGL-Render-Project} ${OGL_RENDER_SOURCES})
target_include_directories(${OpenGL-Render-Project} PUBLIC
					  ../../include/
					  )

if(UNIX)
    target_include_directories(${OpenGL-Render-Project} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${OpenGL-Render-Project} glfw
                                         ${FREETYPE_LIBRARIES}
                                         assimp
                                         sndfile
                                         portaudio
                                         ${CMAKE_DL_LIBS})
else()
    target_include_directories(${OpenGL-Render-Project} PUBLIC "D:/noam4/Libraries/VS/include")
    target_link_libraries(${OpenGL-Render-Project} ${Lib}glfw3.lib
                                         ${Lib}freetype.lib
                                         ${Lib}assimp-vc140-mt.lib
                                         ${Lib}portaudio_x64.lib
                                         ${Lib}sndfile.lib
                                        bufferoverflowU.lib)
endif()			
