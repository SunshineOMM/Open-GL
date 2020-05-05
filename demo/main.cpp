#include <cmath>          // cos, sin
#include <fstream>        // ifstream
#include <iostream>       // cout
#include <random>         // default_random_engine, uniform_real_distribution
#include <sstream>        // stringstream
#include <stdexcept>      // runtime_error
#include <vector>         // vector

#include <GL/glew.h>      // Библиотеки GLEW и OpenGL
#include <GLFW/glfw3.h>   // Библиотека GLFW

using namespace std;

// =============================
// ОБРАБОТЧИК СОБЫТИЙ КЛАВИАТУРЫ
// =============================

static void KeyCallback(GLFWwindow* const window, const int key, const int, const int action, const int) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

// ===================================================================
// ВСПОМОГАТЕЛЬНЫЙ КЛАСС ДЛЯ КОМПИЛЯЦИИ И ЛИНКОВКИ ШЕЙДЕРНОЙ ПРОГРАММЫ
// ===================================================================

class ShaderProgram {
private:
    static GLuint CompileShader(const string& path, const GLenum shaderType) {
        string sourceCode;
        {
            ifstream fs(path);
            stringstream ss;
            ss << fs.rdbuf();
            if (fs.fail()) {
                stringstream errorMessage;
                errorMessage << "Failed to load shader from file!" << endl;
                errorMessage << "Shader source file: \"" << path << "\".";
                throw runtime_error(errorMessage.str().c_str());
            }
            sourceCode = ss.str();
        }

        auto id = glCreateShader(shaderType);
        if (id == 0) {
            stringstream errorMessage;
            errorMessage << "Failed to create shader with type " << shaderType << "!" << endl;
            errorMessage << "Shader source file: \"" << path << "\".";
            throw runtime_error(errorMessage.str().c_str());
        }

        const auto shaderPtr = sourceCode.c_str();
        glShaderSource(id, 1, &shaderPtr, nullptr);

        glCompileShader(id);
        GLint success = GL_FALSE;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            GLint logLength = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
            vector<GLchar> log(logLength);
            glGetShaderInfoLog(id, logLength, nullptr, log.data());
            glDeleteShader(id);
            stringstream errorMessage;
            errorMessage << "Failed to compile shader!" << endl;
            errorMessage << "Shader source file: \"" << path << "\"." << endl;
            if (!log.empty()) {
                errorMessage << log.data();
            }
            throw runtime_error(errorMessage.str().c_str());
        }

        return id;
    }

public:
    const GLuint id;

    ShaderProgram(std::initializer_list<std::pair<std::string, GLenum>> shaders) : id(glCreateProgram()) {
        if (id == 0)
            throw runtime_error("Failed to create shader program!");

        vector<GLuint> compiledShaders;
        compiledShaders.reserve(shaders.size());
        try {
            for (auto& shader : shaders) {
                const auto shaderId = CompileShader(shader.first, shader.second);
                compiledShaders.push_back(shaderId);
                glAttachShader(id, shaderId);
            }
            glLinkProgram(id);

            GLint success = GL_FALSE;
            glGetProgramiv(id, GL_LINK_STATUS, &success);
            if (success == GL_FALSE) {
                GLint length = 0;
                glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
                vector<GLchar> log(length);
                glGetProgramInfoLog(id, length, nullptr, log.data());
                stringstream ess;
                ess << "Failed to link program!" << endl;
                ess << log.data();
                throw runtime_error(ess.str().c_str());
            }

            for (auto shaderId : compiledShaders)
                glDeleteShader(shaderId);
        }
        catch (...) {
            for (auto shaderId : compiledShaders)
                glDeleteShader(shaderId);
            glDeleteProgram(id);
            throw;
        }
    }

    ShaderProgram(const ShaderProgram&) = delete;

    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ~ShaderProgram() {
        glDeleteShader(id);
    }
};

// ================
// НАСТРОЙКИ МОДЕЛИ
// ================

const size_t N = 100;                                 // Количество точек
const GLfloat MIN_SPEED = 0.5f;                       // Минимальная скорость
const GLfloat MAX_SPEED = 0.8f;                       // Максимальная скорость
const GLfloat POINT_SIZE = 1;                         // Размер отрисовываемых точек
const GLfloat BACKGROUND_COLOR[] = { 0.5f, 0, 0.5f }; // Цвет фона
const GLfloat POINT_COLOR[] = { 1, 0.5f, 1 };         // Цвет точек

// Модель одной точки:
// float t0 - время начала движения
// vec2  p0 - начальные координаты
// vec2  v  - вектор скорости


const auto STRIDE = 6 * sizeof(GLfloat); // С учётом выравнивания по модели std430

// ===========
// ТОЧКА ВХОДА
// ===========

int main() {
    try {
        // =============
        // СОЗДАНИЕ ОКНА
        // =============

        if (glfwInit() == GLFW_FALSE)
            throw runtime_error("Failed to initialize GLFW!");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        GLFWwindow* const window = glfwCreateWindow(200, 200, "Demo", nullptr, nullptr);;
        if (window == nullptr)
            throw runtime_error("Failed to create window!");

        glfwSetKeyCallback(window, KeyCallback);
        glfwMakeContextCurrent(window);

        {
            const auto glewStatus = glewInit();
            if (glewStatus != GLEW_OK) {
                stringstream errorMessage;
                errorMessage << "Failed to initialize GLEW!" << endl;
                errorMessage << reinterpret_cast<const char*>(glewGetErrorString(glewStatus));
                throw runtime_error(errorMessage.str().c_str());
            }

            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            glViewport(0, 0, w, h);
        }

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_PROGRAM_POINT_SIZE);

        // =========================
        // СБОРКА ШЕЙДЕРНЫХ ПРОГРАММ
        // =========================

        ShaderProgram initProgram({ {"shaders/init.glsl", GL_COMPUTE_SHADER} });

        ShaderProgram reflectProgram({ {"shaders/reflect.glsl", GL_COMPUTE_SHADER} });

        ShaderProgram renderProgram(
            {
                {"shaders/vertex.glsl",   GL_VERTEX_SHADER},
                {"shaders/fragment.glsl", GL_FRAGMENT_SHADER}
            });

        // ============================================
        // ПОЛУЧЕНИЕ ИДЕНТИФИКАТОРОВ UNIFORM-ПЕРЕМЕННЫХ
        // ============================================

        const auto initMinSpeedLocation = glGetUniformLocation(initProgram.id, "minSpeed");
        const auto initMaxSpeedLocation = glGetUniformLocation(initProgram.id, "maxSpeed");
        const auto initTimeLocation = glGetUniformLocation(initProgram.id, "t");
        if (initMinSpeedLocation == -1 || initMaxSpeedLocation == -1 || initTimeLocation == -1)
            throw runtime_error("Failed to locate init uniform(s)!");

        const auto reflectTimeLocation = glGetUniformLocation(reflectProgram.id, "t");
        if (reflectTimeLocation == -1)
            throw runtime_error("Failed to locate reflect uniform(s)!");

        const auto renderPointSizeLocation = glGetUniformLocation(renderProgram.id, "pointSize");
        const auto renderTLocation = glGetUniformLocation(renderProgram.id, "t");
        const auto renderColorLocation = glGetUniformLocation(renderProgram.id, "color");
        if (renderPointSizeLocation == -1 || renderTLocation == -1 || renderColorLocation == -1)
            throw runtime_error("Failed to locate render uniform(s)!");

        // =================================
        // СОЗДАНИЕ VBO НЕОБХОДИМОГО РАЗМЕРА
        // =================================

        GLuint vbo;
        {
            glGenBuffers(1, &vbo);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo); // Привязка VBO к вычислительному шейдеру
            glBindBuffer(GL_ARRAY_BUFFER, vbo);                 // Привязка VBO к вершинному шейдеру
            vector<GLbyte> data(N * STRIDE);
            glBufferData(GL_ARRAY_BUFFER, N * STRIDE, data.data(), GL_DYNAMIC_COPY);

        }
        // ================================================
        // ЗАПУСК ИНИЦИАЛИЗИРУЮЩЕГО ВЫЧИСЛИТЕЛЬНОГО ШЕЙДЕРА
        // ================================================

        glUseProgram(initProgram.id);
        glUniform1f(initMinSpeedLocation, MIN_SPEED);
        glUniform1f(initMaxSpeedLocation, MAX_SPEED);
        glUniform1f(initTimeLocation, static_cast<GLfloat>(glfwGetTime()));
        glDispatchCompute(N, 1, 1);
        //glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glUseProgram(0);
        glDeleteProgram(initProgram.id);

        // ============================
        // СОЗДАНИЕ И ИНИЦИАЛИЗАЦИЯ VAO
        // ============================

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        {
            const auto locationT0 = glGetAttribLocation(renderProgram.id, "t0");
            const auto locationP0 = glGetAttribLocation(renderProgram.id, "p0");
            const auto locationV = glGetAttribLocation(renderProgram.id, "v");
            if (locationT0 == -1 || locationP0 == -1 || locationV == -1)
                throw runtime_error("Failed to locate render attribute(s)!");

            glVertexAttribPointer(locationT0, 1, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(0 * sizeof(GLfloat)));
            glVertexAttribPointer(locationP0, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(2 * sizeof(GLfloat)));
            glVertexAttribPointer(locationV, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(4 * sizeof(GLfloat)));

            glEnableVertexAttribArray(locationT0);
            glEnableVertexAttribArray(locationP0);
            glEnableVertexAttribArray(locationV);
        }

        // ===========================
        // ПРЕДСТАРТОВАЯ ПОДГОТОВКА...
        // ===========================

        glClearColor(BACKGROUND_COLOR[0], BACKGROUND_COLOR[1], BACKGROUND_COLOR[2], 1);

        glUseProgram(renderProgram.id);
        glUniform1f(renderPointSizeLocation, POINT_SIZE);
        glUniform3fv(renderColorLocation, 1, POINT_COLOR);
        glUseProgram(0);

        // ========
        // ПОЕХАЛИ!
        // ========

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            const auto time = static_cast<GLfloat>(glfwGetTime());

            // Мы пропускаем привязку VBO к GL_SHADER_STORAGE_BUFFER,
            // т. к. у нас только один VBO и он уже привязан к GL_SHADER_STORAGE_BUFFER
            glUseProgram(reflectProgram.id);
            glUniform1f(reflectTimeLocation, time);
            glDispatchCompute(N, 1, 1);
            glUseProgram(0);

            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

            // Мы пропускаем привязку VBO к GL_ARRAY_BUFFER,
            // т. к. у нас только один VBO и он уже привязан к GL_ARRAY_BUFFER
            glUseProgram(renderProgram.id);
            glUniform1f(renderTLocation, time);
            glDrawArrays(GL_POINTS, 0, N);
            glUseProgram(0);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    catch (const exception& e) {
        // ================
        // ОБРАБОТКА ОШИБОК
        // ================

        glfwTerminate();
        cout << e.what() << endl;
        return EXIT_FAILURE;
    }
}
