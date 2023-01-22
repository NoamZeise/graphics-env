add_executable(MyApp app.cpp main.cpp)

target_link_libraries(MyApp GameHelper Graphics)
target_include_directories(MyApp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}../include")

add_custom_command(TARGET MyApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../resources"
        $<TARGET_FILE_DIR:MyApp>)
