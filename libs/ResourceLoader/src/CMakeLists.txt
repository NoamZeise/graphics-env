add_library(ResourceLoader resources/stb_image_impl.cpp)
target_include_directories(ResourceLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
