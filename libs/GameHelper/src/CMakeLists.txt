add_library(GameHelper INTERFACE)

target_include_directories(GameHelper INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
  target_link_libraries(GameHelper INTERFACE sndfile portaudio) 
elseif(WIN32)
  target_include_directories(GameHelper INTERFACE "D:/noam4/Libraries/VS/include")
  target_link_libraries(GameHelper INTERFACE
    "D:/noam4/Libraries/VS/lib/x64/sndfile.lib"
    "D:/noam4/Libraries/VS/lib/x64/portaudio_x64.lib") 
endif()
