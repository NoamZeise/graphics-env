function(add_example name sourcefile)
  add_executable(${name} ${sourcefile})
  target_link_libraries(${name} graphics-env)
endfunction()

add_example(minimum minimum.cpp)
if((NOT NO_ASSIMP) AND (NOT NO_FREETYPE))
  if(NOT NO_AUDIO)
    add_example(basic basic.cpp)
    add_example(switch_scenes switch_scenes.cpp)
  endif()
  add_example(asset_pools asset_pools.cpp)
  add_example(animated_models animated_models.cpp)
endif()

# only need to copy files for one example
# as they are all built in the same folder
set(exec-name minimum)
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy-libs.cmake")
# copy resources folder to built executable path
add_custom_command(TARGET ${exec-name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_LIST_DIR}/../resources"
    $<TARGET_FILE_DIR:${exec-name}>)
