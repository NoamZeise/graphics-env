find_package(sndfile REQUIRED)
find_package(portaudio REQUIRED)

add_library(GameHelper camera.cpp)

target_include_directories(GameHelper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_dependencies(GameHelper glm glfw)
target_link_libraries(GameHelper PUBLIC sndfile portaudio glm glfw)
