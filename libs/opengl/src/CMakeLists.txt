file(GLOB OGL_RENDER_SOURCES
  *
  resources/*
  parts/*
)
add_library(opengl-env ${OGL_RENDER_SOURCES})

add_dependencies(opengl-env resource-loader graphics-api glfw)
target_link_libraries(opengl-env PRIVATE resource-loader)
target_link_libraries(opengl-env PUBLIC graphics-api glfw)

target_include_directories(opengl-env PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../glad-include/
)
