add_library(Graphics render.cpp)

add_dependencies(Graphics OutfacingGraphics)
target_link_libraries(Graphics PUBLIC OutfacingGraphics)
if(NOT NO_VULKAN)
  add_dependencies(Graphics VulkanEnvironment)
  target_link_libraries(Graphics PRIVATE VulkanEnvironment)
endif()
if(NOT NO_OPENGL)
  add_dependencies(Graphics OpenGL-Environment)
  target_link_libraries(Graphics PRIVATE OpenGL-Environment)
endif()

target_include_directories(Graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
