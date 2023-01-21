add_library(Graphics render.cpp)

add_dependencies(Graphics VulkanEnvironment OpenGL-Environment OutfacingGraphics)

target_link_libraries(Graphics PRIVATE VulkanEnvironment OpenGL-Environment)
target_link_libraries(Graphics PUBLIC OutfacingGraphics)

target_include_directories(Graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
