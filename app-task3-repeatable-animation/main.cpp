#include <iostream>       // cout
#include <stdexcept>      // runtime_error
#include <vector>
#include <core.h>
#include <time.h>
#include <cstdlib>
#include <cmath>          // cos, sin

using namespace std;


// Функция, которая будет обрабатывать ошибки GLFW.
void ErrorCallback(const int code, const char* const error) {
	throw runtime_error(error);
}



//=====================================================================================================================
//															ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
//=====================================================================================================================

// Модель одной точки:
// float t0 - время начала движения
// vec2  p0 - начальные координаты
// vec2  v  - вектор скорости
// vec3 color - вектор цвета

const GLfloat MIN_SPEED = 0.5f;                       // Минимальная скорость
const GLfloat MAX_SPEED = 0.8f;                       // Максимальная скорость
const GLfloat BACKGROUND_COLOR[] = { 0.5f, 0, 0.5f }; // Цвет фона
const size_t N = 100; // Количество точек

const auto STRIDE = 12 * sizeof(GLfloat); // С учётом выравнивания по модели std430
const GLfloat POINT_SIZE = 3;  // Размер отрисовываемых точек
const GLfloat POINT_COLOR[] = { 1, 0.5f, 1 };         // Цвет точек

struct Rgb {
	GLint _r;
	GLint _g;
	GLint _b;
	Rgb(GLint r, GLint g, GLint b) {
		_r = r;
		_g = g;
		_b = b;
	}
};
const vector<Rgb> RGB_VEC = { *new Rgb(255,0,255),*new Rgb(255,255,0),*new Rgb(0,255,0),*new Rgb(255,0,0),*new Rgb(0,255,255) };



int main() {
	srand(time(NULL));

	// Устанавливаем функцию обработки ошибок GLFW.
	glfwSetErrorCallback(ErrorCallback);
	try {
		if (glfwInit() == GLFW_FALSE) throw runtime_error("Failed to init GLFW!");

		// Создание окна
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		GLFWwindow* window;
		{
			auto monitor = glfwGetPrimaryMonitor();
			auto mode = glfwGetVideoMode(monitor);
			window = glfwCreateWindow(mode->width, mode->height, "TITLE", nullptr, nullptr);
			if (window == nullptr)
				throw runtime_error("Failed to create window!");
			glfwSetKeyCallback(window, KeyCallback);
			glfwMakeContextCurrent(window);

			int w, h;
			glfwGetFramebufferSize(window, &w, &h);

			glViewport(0, 0, w, h);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		// Устанавливаем функции обработки событий клавиатуры  
		glfwSetKeyCallback(window, KeyCallback);


		// Делаем созданное окно текущим.
		// Все дальнейшие вызываемые функции OpenGL будут относиться к данному окну.
		glfwMakeContextCurrent(window);

		// Инициализируем GLEW.
		// Происходит инициализация функций OpenGL.
		{
			auto error = glewInit();
			if (error != GLEW_OK) {

				auto message = reinterpret_cast<const char*>(glewGetErrorString(error)); throw runtime_error(string("Failed to initialize GLEW: ") + message);

			}

		}
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_PROGRAM_POINT_SIZE);

		//место для кода отрисовки точек



		// =========================
		// СБОРКА ШЕЙДЕРНЫХ ПРОГРАММ
		// =========================

		ShaderProgram renderProgram({ {"shaders/vertex.glsl",   GL_VERTEX_SHADER},{"shaders/fragment.glsl", GL_FRAGMENT_SHADER} });

		ShaderProgram initProgram({ {"shaders/init.glsl", GL_COMPUTE_SHADER} });

		ShaderProgram reflectProgram({ {"shaders/reflect.glsl", GL_COMPUTE_SHADER} });

		// ============================================
		// ПОЛУЧЕНИЕ ИДЕНТИФИКАТОРОВ UNIFORM-ПЕРЕМЕННЫХ
		// ============================================

		const auto renderPointSizeLocation = glGetUniformLocation(renderProgram.id, "pointSize");
		const auto renderPointTimeLocation = glGetUniformLocation(renderProgram.id, "pointTime");
		//const auto renderColorLocation = glGetUniformLocation(renderProgram.id, "color");
		if (renderPointSizeLocation == -1 || renderPointTimeLocation == -1 )//|| renderColorLocation==-1)
			throw runtime_error("Failed to locate render uniform(s)!");

		const auto reflectPointTimeLocation = glGetUniformLocation(reflectProgram.id, "pointTime");
		if (reflectPointTimeLocation == -1)
			throw runtime_error("Failed to locate init uniform(s)!");

		const auto initMinSpeedLocation = glGetUniformLocation(initProgram.id, "minSpeed");
		const auto initMaxSpeedLocation = glGetUniformLocation(initProgram.id, "maxSpeed");
		const auto initPointTimeLocation = glGetUniformLocation(initProgram.id, "pointTime");;
		if (initMinSpeedLocation == -1 || initMaxSpeedLocation == -1 || initPointTimeLocation == -1 )
			throw runtime_error("Failed to locate init uniform(s)!");


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
		glUniform1f(initPointTimeLocation, static_cast<GLfloat>(glfwGetTime()));
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
			const auto locationC= glGetAttribLocation(renderProgram.id, "color");
			if (locationT0 == -1 || locationP0 == -1 || locationV == -1 || locationC==-1)
				throw runtime_error("Failed to locate render attribute(s)!");

			glVertexAttribPointer(locationT0, 1, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(0 * sizeof(GLfloat)));
			glVertexAttribPointer(locationP0, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));
			glVertexAttribPointer(locationV, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(6 * sizeof(GLfloat)));
			glVertexAttribPointer(locationC, 3, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(9 * sizeof(GLfloat)));

			glEnableVertexAttribArray(locationT0);
			glEnableVertexAttribArray(locationP0);
			glEnableVertexAttribArray(locationV);
			glEnableVertexAttribArray(locationC);
		}


		// ===========================
		// ПРЕДСТАРТОВАЯ ПОДГОТОВКА...
		// ===========================
		glClearColor(255, 255, 255, 1);

		glUseProgram(renderProgram.id);
		glUniform1f(renderPointSizeLocation, POINT_SIZE);
		//glUniform3fv(renderColorLocation, 1, POINT_COLOR);
		glUseProgram(0);

		// ========
		// ПОЕХАЛИ!
		// ========
		while (!glfwWindowShouldClose(window)) {
			// Очищаем экран.
			glClear(GL_COLOR_BUFFER_BIT);

			const auto time = static_cast<GLfloat>(glfwGetTime());

			// Мы пропускаем привязку VBO к GL_SHADER_STORAGE_BUFFER,
		   // т. к. у нас только один VBO и он уже привязан к GL_SHADER_STORAGE_BUFFER
			glUseProgram(reflectProgram.id);
			glUniform1f(reflectPointTimeLocation, time);
			glDispatchCompute(N, 1, 1);
			glUseProgram(0);

			glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

			// Мы пропускаем привязку VBO к GL_ARRAY_BUFFER,
		   // т. к. у нас только один VBO и он уже привязан к GL_ARRAY_BUFFER
			glUseProgram(renderProgram.id);
			glUniform1f(renderPointTimeLocation, time);
			glDrawArrays(GL_POINTS, 0, N);
			glUseProgram(0);


			glfwSwapBuffers(window);
			// Блокируем выполнение текущего потока, пока не произойдёт какого-либо события.
			// Если необходимо отрисовывать следующий кадр, не дожидаясь событий,
			// используйте glfwPollEvents() вместо glfwWaitEvents().
			glfwPollEvents();

		}

	}
	catch (const exception& e) {
		glfwTerminate();
		cout << e.what() << endl;		//ЗАПОМНИТЬ!!!
		return EXIT_FAILURE;
	}
}
