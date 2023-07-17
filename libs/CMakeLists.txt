add_subdirectory(Graphics)
if(NOT NO_AUDIO)
  add_subdirectory(audio)
endif()
add_subdirectory(GameHelper)
add_subdirectory(VulkanEnvironment)
add_subdirectory(OpenGLEnvironment)

