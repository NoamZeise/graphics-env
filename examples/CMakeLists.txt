function(add_example name sourcefile)
  add_executable(${name} ${sourcefile})
  target_link_libraries(${name} graphics-env)
endfunction()

add_example(minimum minimum.cpp)
if((NOT NO_ASSIMP) AND (NOT NO_FREETYPE))
  if(NOT NO_AUDIO)
    add_example(basic basic.cpp)
    #add_example(switch_scenes switch_scenes.cpp)
  endif()
  #add_example(asset_pools asset_pools.cpp)
  #add_example(animated_models animated_models.cpp)
  #add_example(gooch_shading gooch_shading.cpp)
endif()

# Copy files for one executable - ie libs/resource files
# this does mean only `minimum` will get built statically
set(exec-name minimum)
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy-libs.cmake")
# copy resources folder to built executable path
add_custom_command(TARGET ${exec-name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_LIST_DIR}/../resources"
    $<TARGET_FILE_DIR:${exec-name}>)
add_custom_command(TARGET ${exec-name} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_LIST_DIR}/shaders/"
  $<TARGET_FILE_DIR:${exec-name}>)
