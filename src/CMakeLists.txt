add_library(GraphicsEnv manager.cpp)
add_dependencies(GraphicsEnv Graphics)
target_link_libraries(GraphicsEnv PUBLIC GameHelper Graphics)
target_include_directories(GraphicsEnv PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")


