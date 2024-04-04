# expects "exec-name" = name of your executable
# ie do "set(exec-name name-of-your-executable)"
# before including this file (like in examples/CMakeLists.txt)

# disable console window in release mode
# build static when defined
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # get linux binary to check current dir for libraries
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath='${ORIGIN}'")
  target_link_libraries(${exec-name} -pthread)
  if(GRAPHICS_BUILD_STATIC)
    target_link_libraries(${exec-name} -static-libgcc -static-libstdc++)
  endif()
  if(CMAKE_BUILD_TYPE STREQUAL "Release" AND MINGW)
    target_link_libraries(${exec-name} -mwindows)
  endif()
elseif(MSVC)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(${exec-name} PRIVATE "/SUBSYSTEM:WINDOWS")
    target_link_options(${exec-name} PRIVATE "/ENTRY:mainCRTStartup")
  endif()
endif()

# Copy any shared libs built by this project
# Into the same folder as the built executable
  
if(NOT NO_ASSIMP)
  if(NOT BUILD_ASSIMP_STATIC)
    # Assimp
    add_custom_command(TARGET ${exec-name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      $<TARGET_FILE_DIR:assimp>
      $<TARGET_FILE_DIR:${exec-name}>)
  endif()
endif()

if(NOT NO_FREETYPE)
  if(NOT VKENV_BUILD_STATIC)
    # Freetype
    add_custom_command(TARGET ${exec-name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      $<TARGET_FILE_DIR:freetype>
      $<TARGET_FILE_DIR:${exec-name}>)
  endif()
endif()

if(NOT VKENV_BUILD_STATIC)
  # GLFW
  add_custom_command(TARGET ${exec-name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    $<TARGET_FILE_DIR:glfw>
    $<TARGET_FILE_DIR:${exec-name}>)
  # GLM
  add_custom_command(TARGET ${exec-name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    $<TARGET_FILE_DIR:glm>
    $<TARGET_FILE_DIR:${exec-name}>)
endif()
