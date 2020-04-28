cmake_minimum_required(VERSION 3.8)

project("OpenGL")

# Выключение некоторых предупреждений для подключаемых библиотек
if (MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_link_options(/IGNORE:4281)
endif ()

# Подключение библиотеки GLEW 2.1.0
set(OpenGL_GL_PREFERENCE GLVND)
add_subdirectory("glew/build/cmake")
target_compile_definitions(glew_s PUBLIC GLEW_NO_GLU)
target_include_directories(glew_s PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glew/include>")

# Подключение библиотеки GLFW 3.3.2
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("glfw")

# Включение высокого уровня предупреждений для нашего кода
if (MSVC)
    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif ()
elseif (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif ()

# Подключение собственной вспомогательной библиотеки
add_subdirectory("app-core")

# Подключение пустых заготовок проектов для выполнения лабораторной работы
add_subdirectory("app-task1-no-animation")
add_subdirectory("app-task2-one-shot-animation")
add_subdirectory("app-task3-repeatable-animation")
add_subdirectory("app-task4-surprise-your-teacher")

# Проект для бесстыдного подглядывания...
add_subdirectory("demo")
