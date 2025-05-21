add_library(graphics-env
  # graphics
  manager.cpp
  model_gen.cpp
  # game
  camera.cpp
  keyboard.cpp
  mouse.cpp
  controller.cpp
  timer.cpp
  random.cpp
)

add_dependencies(graphics-env render-api)
target_link_libraries(graphics-env PUBLIC render-api glfw)

if(NOT NO_VULKAN)
  add_dependencies(graphics-env vulkan-env)
  target_link_libraries(graphics-env PRIVATE vulkan-env)
endif()
if(NOT NO_OPENGL)
  add_dependencies(graphics-env opengl-env)
  target_link_libraries(graphics-env PRIVATE opengl-env)
endif()
if(NOT NO_AUDIO)
  add_dependencies(graphics-env audio)
  target_link_libraries(graphics-env PUBLIC audio)
endif()

target_include_directories(graphics-env PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")
