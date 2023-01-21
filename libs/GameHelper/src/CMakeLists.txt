add_library(GameHelper camera.cpp)

target_include_directories(GameHelper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  target_link_libraries(GameHelper PUBLIC sndfile portaudio glfw) 
elseif(WIN32)
  target_include_directories(GameHelper PUBLIC "D:/noam4/Libraries/VS/include")
  target_link_libraries(GameHelper PRIVATE
    glfw
    "D:/noam4/Libraries/VS/lib/x64/sndfile.lib"
    "D:/noam4/Libraries/VS/lib/x64/portaudio_x64.lib") 
endif()
