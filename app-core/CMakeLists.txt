cmake_minimum_required (VERSION 3.8)

project("AppCore" CXX)

add_library (${PROJECT_NAME}
    "include/core.h"
    "src/key_callback.cpp"
    "src/shader_program.cpp")

target_include_directories (${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} glew_s glfw)
