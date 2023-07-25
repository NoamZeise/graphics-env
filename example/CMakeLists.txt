add_executable(MyApp main.cpp app.cpp)
target_link_libraries(MyApp GraphicsEnv)

if(UNIX) # want linux binary to check current dir for libraries
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath='${ORIGIN}'")
    if(GRAPHICS_BUILD_STATIC)
      target_link_libraries(MyApp -static-libgcc -static-libstdc++)
    endif()
  endif()
  target_link_libraries(MyApp -pthread)
endif()

add_custom_command(TARGET MyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../resources"
        $<TARGET_FILE_DIR:MyApp>)

if(NOT NO_ASSIMP)
  if(NOT BUILD_ASSIMP_STATIC)	
    add_custom_command(TARGET MyApp POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      $<TARGET_FILE_DIR:assimp>
      $<TARGET_FILE_DIR:MyApp>)
  endif()
endif()

if(NOT NO_FREETYPE)
  if(NOT VKENV_BUILD_STATIC)
    add_custom_command(TARGET MyApp POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      $<TARGET_FILE_DIR:freetype>
      $<TARGET_FILE_DIR:MyApp>)
  endif()
endif()

if(NOT VKENV_BUILD_STATIC)
  add_custom_command(TARGET MyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    $<TARGET_FILE_DIR:glfw>
    $<TARGET_FILE_DIR:MyApp>)
endif()


add_executable(minimum minimum.cpp)
target_link_libraries(minimum GraphicsEnv)
