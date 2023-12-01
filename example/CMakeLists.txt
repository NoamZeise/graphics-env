add_executable(MyApp main.cpp app.cpp)
target_link_options(MyApp PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(MyApp GraphicsEnv)

# only need to copy files for one example
# as they are all built in the same folder
set(exec-name MyApp)
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/copy-resources.cmake")

add_executable(minimum minimum.cpp)
target_link_options(minimum PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(minimum GraphicsEnv)

add_executable(assetPools asset_pools.cpp)
target_link_options(assetPools PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})
target_link_libraries(assetPools GraphicsEnv)

