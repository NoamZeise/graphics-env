add_library(render-internal
    stb_image_impl.cpp
    texture_loader.cpp
    model_loader.cpp
    assimp_loader.cpp
    shader_buffers.cpp
)

target_sources(render-internal PRIVATE font_loader.cpp)

if(NOT NO_FREETYPE)
  target_link_libraries(render-internal PRIVATE freetype)
  target_include_directories(render-internal PRIVATE ${FREETYPE_INCLUDE_DIRS})
endif()
if(NOT NO_ASSIMP)
  target_link_libraries(render-internal PUBLIC assimp)
endif()
add_dependencies(render-internal render-api)
target_link_libraries(render-internal PUBLIC render-api)

target_include_directories(render-internal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
