#include <core.h>

// Обработчик событий клавиатуры.
void KeyCallback(GLFWwindow* const window, const int key, const int scancode, const int action, const int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);  
}
