add_library(GraphicsEnv
  manager.cpp
  model_gen.cpp
)
add_dependencies(GraphicsEnv GameHelper OutfacingGraphics)
target_link_libraries(GraphicsEnv PUBLIC GameHelper OutfacingGraphics)
if(NOT NO_VULKAN)
  add_dependencies(GraphicsEnv VulkanEnvironment)
  target_link_libraries(GraphicsEnv PRIVATE VulkanEnvironment)
endif()
if(NOT NO_OPENGL)
  add_dependencies(GraphicsEnv OpenGL-Environment)
  target_link_libraries(GraphicsEnv PRIVATE OpenGL-Environment)
endif()
target_include_directories(GraphicsEnv PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")


