add_executable(minimum minimum.cpp)
target_link_options(minimum PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(minimum GraphicsEnv)

add_executable(asset_pools asset_pools.cpp)
target_link_options(asset_pools PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(asset_pools GraphicsEnv)

add_executable(switch_scenes switch_scenes.cpp)
target_link_options(switch_scenes PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(switch_scenes GraphicsEnv)

add_executable(animated_models animated_models.cpp)
target_link_options(animated_models PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(animated_models GraphicsEnv)

# only need to copy files for one example
# as they are all built in the same folder
set(exec-name minimum)
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy-resources.cmake")

add_custom_command(TARGET ${exec-name} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_LIST_DIR}/../resources"
  $<TARGET_FILE_DIR:${exec-name}>)
