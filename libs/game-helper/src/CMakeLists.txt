add_library(game-helper
  camera.cpp
  keyboard.cpp
  mouse.cpp
  controller.cpp
  timer.cpp
  random.cpp
)
target_include_directories(game-helper PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
add_dependencies(game-helper glm glfw)
target_link_libraries(game-helper PUBLIC glm glfw)
if(NOT NO_AUDIO)
  add_dependencies(game-helper audio)
  target_link_libraries(game-helper PUBLIC audio)
endif()
