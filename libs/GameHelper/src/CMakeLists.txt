add_library(GameHelper INTERFACE)

target_include_directories(GameHelper INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GameHelper INTERFACE sndfile portaudio) 
