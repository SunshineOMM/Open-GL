cmake_minimum_required (VERSION 3.8)

project("AppTask2" CXX)

add_executable (${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} AppCore)

# При каждой сборке:
# 1. Удаляем старую папку с шейдерами в директории с исполняемым файлом.
# 2. Копируем папку с шейдерами из директории с исходным кодом в директорию с исполняемым файлом.
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory
    "${CMAKE_CURRENT_BINARY_DIR}/shaders")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "${CMAKE_CURRENT_BINARY_DIR}/shaders")
