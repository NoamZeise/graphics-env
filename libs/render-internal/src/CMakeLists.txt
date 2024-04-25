add_library(render-internal
    stb_image_impl.cpp
    vertex_model.cpp
    texture_loader.cpp
    model_loader.cpp
    assimp_loader.cpp
)

target_sources(render-internal PRIVATE font_loader.cpp)

if(NOT NO_FREETYPE)
  add_dependencies(render-internal freetype)
  target_link_libraries(render-internal PRIVATE freetype)
endif()
if(NOT NO_ASSIMP)
  target_link_libraries(render-internal PUBLIC assimp)
endif()
add_dependencies(render-internal render-api)
target_link_libraries(render-internal PUBLIC render-api)

target_include_directories(render-internal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
