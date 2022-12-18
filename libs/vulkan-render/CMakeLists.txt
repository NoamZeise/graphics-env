file(GLOB VK_RENDER_SOURCES
	  *
	  resources/*
	  parts/*
	  )

set(Vulkan-Render-Project "Vulkan-Renderer")

add_library(${Vulkan-Render-Project} ${VK_RENDER_SOURCES})
target_include_directories(${Vulkan-Render-Project} PUBLIC
					  ../../include/
					  )

if(UNIX)
    target_include_directories(${Vulkan-Render-Project} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${Vulkan-Render-Project} glfw
                                         ${FREETYPE_LIBRARIES}
                                         assimp
                                         sndfile
                                         portaudio
                                         ${CMAKE_DL_LIBS})
else()
    target_include_directories(${Vulkan-Render-Project} PUBLIC "D:/noam4/Libraries/VS/include")
    target_link_libraries(${Vulkan-Render-Project} ${Lib}glfw3.lib
                                         ${Lib}freetype.lib
                                         ${Lib}vulkan-1.lib
                                         ${Lib}assimp-vc140-mt.lib
                                         ${Lib}portaudio_x64.lib
                                         ${Lib}sndfile.lib
                                        bufferoverflowU.lib)
endif()			
