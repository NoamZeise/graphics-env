add_library(GameHelper
  camera.cpp
  keyboard.cpp
  mouse.cpp
)
target_include_directories(GameHelper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_dependencies(GameHelper glm glfw)
target_link_libraries(GameHelper PUBLIC glm glfw)
