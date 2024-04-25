add_library(render-api animation.cpp)
add_dependencies(render-api glm)
target_link_libraries(render-api PUBLIC glm)
target_include_directories(render-api PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
