add_library(render-api animation.cpp default_vertex_types.cpp pipeline.cpp)
add_dependencies(render-api glm::glm)
target_link_libraries(render-api PUBLIC glm::glm)
target_include_directories(render-api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
