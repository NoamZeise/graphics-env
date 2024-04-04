add_library(graphics-env
  manager.cpp
  model_gen.cpp
)

add_dependencies(graphics-env game-helper graphics-api)
target_link_libraries(graphics-env PUBLIC
  game-helper graphics-api)

if(NOT NO_VULKAN)
  add_dependencies(graphics-env vulkan-env)
  target_link_libraries(graphics-env PRIVATE vulkan-env)
endif()
if(NOT NO_OPENGL)
  add_dependencies(graphics-env opengl-env)
  target_link_libraries(graphics-env PRIVATE opengl-env)
endif()

target_include_directories(graphics-env PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")
