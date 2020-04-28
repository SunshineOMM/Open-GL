#include <iostream>       // cout
#include <stdexcept>      // runtime_error

#include <core.h>

using namespace std;

int main() {
    try {
        // Основной код приложения пишите здесь
        // Кидайте runtime_error("Exception message") в случае какой-либо ошибки
    }
    catch (const exception& e) {
        glfwTerminate();
        cout << e.what() << endl;
        return EXIT_FAILURE;
    }
}
