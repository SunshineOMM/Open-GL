#include <iostream>       // cout
#include <stdexcept>      // runtime_error
#include <vector>
#include <core.h>
#include <cstdlib>
#include <cmath>          // cos, sin

using namespace std;
//const float PI = 3.141592653589;

// Функция, которая будет обрабатывать ошибки GLFW.
void ErrorCallback(const int code, const char* const error) {
	throw runtime_error(error);
}


//=====================================================================================================================
//															ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
//=====================================================================================================================

// Модель разлетающейся одной точки:
// float t0 - время начала движения
// vec2  p0 - начальные координаты
// vec2  v  - вектор скорости
// vec3 color - вектор цвета

// Модель точки эллипса:
// vec2  p -  координаты центра

// vec3 color - вектор цвета
// vec2 radius



const size_t N = 1000; // Количество точек


const auto STRIDE = 8 * sizeof(GLfloat); // С учётом выравнивания по модели std430
const auto STRIDE_ELLIPSE = 5 * sizeof(GLfloat); // С учётом выравнивания по модели std430
const GLfloat POINT_SIZE = 4;  // Размер отрисовываемых точек

class CircleProgram :public ShaderProgram {
	GLint _slice;
	GLint _cx;
	GLint _cy;
	GLint _rx;
	GLint _ry;
	GLint _pointSize;
	GLint _color;
	GLuint _vbo;
	GLuint _vao;


public:
	CircleProgram() :ShaderProgram({ {"shaders/vertex ellipse.glsl",   GL_VERTEX_SHADER},{"shaders/fragment.glsl", GL_FRAGMENT_SHADER} }) {
		_slice= glGetUniformLocation(id, "slice");
		_cx = glGetUniformLocation(id, "cx");
		_cy = glGetUniformLocation(id, "cy");
		_rx = glGetUniformLocation(id, "rx");
		_ry = glGetUniformLocation(id, "ry");
		_pointSize = glGetUniformLocation(id, "pointSize");		
		_color = glGetUniformLocation(id, "color");
		
		if (_cx == -1 || _cy == -1 || _rx == -1 || _ry==-1 || _pointSize==-1 || _color==-1 || _slice==-1)
			throw runtime_error("Failed to locate renderEllipse uniform(s)!");

		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		GLfloat dummy = 0;
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat), &dummy, GL_STATIC_DRAW);

		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		const auto dummylocation = glGetAttribLocation(id, "dummy");
		glVertexAttribPointer(dummylocation, 1, GL_FLOAT, GL_FALSE, 0,nullptr);
		glEnableVertexAttribArray(dummylocation);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	}
	void Draw(const GLfloat cx, const GLfloat cy, const GLfloat rx, const GLfloat ry, const GLfloat* color ) {
		glUseProgram(id);
		glBindVertexArray(_vao);

		glUniform1f(_slice, 0);
		glUniform1f(_cx, cx);
		glUniform1f(_cy, cy);
		glUniform1f(_rx, rx);
		glUniform1f(_ry, ry);
		glUniform3f(_color, color[0], color[1], color[2]);
		glUniform1f(_pointSize, POINT_SIZE);
		
		
		

		glDrawArrays(GL_LINE_LOOP, 0, 360);


		glBindVertexArray(0);
		glUseProgram(0);
		

	}
	void DrawingASpecificArea(const GLfloat cx, const GLfloat cy, const GLfloat rx, const GLfloat ry, const GLfloat* color, const GLfloat slice) {		
		glUseProgram(id);
		glBindVertexArray(_vao);

		glUniform1f(_slice, slice);
		glUniform1f(_cx, cx);
		glUniform1f(_cy, cy);
		glUniform1f(_rx, rx);
		glUniform1f(_ry, ry);
		glUniform3f(_color, color[0], color[1], color[2]);
		glUniform1f(_pointSize, POINT_SIZE);

		glDrawArrays(GL_LINE_LOOP, 0, 360);


		glBindVertexArray(0);
		glUseProgram(0);
	}
	~CircleProgram() {
		glDeleteVertexArrays(1,&_vao);
		glDeleteBuffers(1,&_vbo);
 
	}
};

class TriangleProgram :public ShaderProgram {	
	GLint _pointSize;
	GLint _color;
	GLuint _vbo;
	GLuint _vao;

public:
	TriangleProgram() :ShaderProgram({ {"shaders/vertex triangle.glsl",   GL_VERTEX_SHADER},{"shaders/fragment.glsl", GL_FRAGMENT_SHADER} }) {

		_pointSize = glGetUniformLocation(id, "pointSize");
		_color = glGetUniformLocation(id, "color");

		if ( _pointSize == -1 || _color == -1)
			throw runtime_error("Failed to locate renderEllipse uniform(s)!");

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}
	void Draw(const GLfloat* p, const GLfloat* color) {
		glUseProgram(id);

		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		vector<GLfloat> data(6);
		for (int i = 0; i < data.size(); ++i) {
			if (i % 2 == 0) {
				data[i] = p[i];
				data[i + 1] = p[i+1];
			}
		}
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), data.data(), GL_STATIC_DRAW);

		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		const auto Plocation = glGetAttribLocation(id, "p");

		glVertexAttribPointer(Plocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(Plocation);

		glBindVertexArray(_vao);

		glUniform3f(_color, color[0], color[1], color[2]);
		glUniform1f(_pointSize, POINT_SIZE);




		glDrawArrays(GL_TRIANGLES, 0, 3);


		glBindVertexArray(0);
		glUseProgram(0);


	}
	~TriangleProgram() {
		glDeleteVertexArrays(1, &_vao);
		glDeleteBuffers(1, &_vbo);

	}
};




int main() {

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
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//место для кода отрисовки точек



		// =========================
		// СБОРКА ШЕЙДЕРНЫХ ПРОГРАММ
		// =========================

		ShaderProgram renderProgram({ {"shaders/vertex.glsl",   GL_VERTEX_SHADER},{"shaders/fragment.glsl", GL_FRAGMENT_SHADER} });

		ShaderProgram initProgram({ {"shaders/init.glsl", GL_COMPUTE_SHADER} });

		ShaderProgram reflectProgram({ {"shaders/reflect.glsl", GL_COMPUTE_SHADER} });

		ShaderProgram renderEllipseProgram({ {"shaders/vertex ellipse.glsl",   GL_VERTEX_SHADER},{"shaders/fragment.glsl", GL_FRAGMENT_SHADER} });

		// ============================================
		// ПОЛУЧЕНИЕ ИДЕНТИФИКАТОРОВ UNIFORM-ПЕРЕМЕННЫХ
		// ============================================

		const auto renderPointSizeLocation = glGetUniformLocation(renderProgram.id, "pointSize");
		const auto renderPointTimeLocation = glGetUniformLocation(renderProgram.id, "pointTime");
		if (renderPointSizeLocation == -1 || renderPointTimeLocation == -1)
			throw runtime_error("Failed to locate render uniform(s)!");

		


		const auto reflectPointTimeLocation = glGetUniformLocation(reflectProgram.id, "pointTime");
		const auto reflectPoint0Location = glGetUniformLocation(reflectProgram.id, "p0");		
		if (reflectPointTimeLocation == -1 || reflectPoint0Location==-1)
			throw runtime_error("Failed to locate reflect uniform(s)!");

		
		const auto initPointTimeLocation = glGetUniformLocation(initProgram.id, "pointTime");
		const auto initPoint0Location = glGetUniformLocation(initProgram.id, "p0");
		if ( initPointTimeLocation == -1 || initPoint0Location==-1)
			throw runtime_error("Failed to locate init uniform(s)!");


		// =================================
		// СОЗДАНИЕ VBO НЕОБХОДИМОГО РАЗМЕРА
		// =================================
		GLuint vbo[] = { 0,0,0 };
		{
			glGenBuffers(2, vbo);

			
			glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);                 // Привязка VBO к вершинному шейдеру
			vector<GLbyte> data1(N * STRIDE);
			glBufferData(GL_ARRAY_BUFFER, N * STRIDE, data1.data(), GL_DYNAMIC_COPY);

			
			glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);                 // Привязка VBO к вершинному шейдеру
			vector<GLbyte> data2(N * STRIDE);
			glBufferData(GL_ARRAY_BUFFER, N * STRIDE, data2.data(), GL_DYNAMIC_COPY);
			

		}


		// ================================================
		// ЗАПУСК ИНИЦИАЛИЗИРУЮЩЕГО ВЫЧИСЛИТЕЛЬНОГО ШЕЙДЕРА
		// ================================================

		glUseProgram(initProgram.id);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo[0]);
		glUniform2f(initPoint0Location, -0.25,0.5);
		glUniform1f(initPointTimeLocation, static_cast<GLfloat>(glfwGetTime()));
		glDispatchCompute(N, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(initProgram.id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo[1]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glUniform2f(initPoint0Location, 0.25, 0.5);

		glDispatchCompute(N, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(0);
		glDeleteProgram(initProgram.id);


		// ============================
		// СОЗДАНИЕ И ИНИЦИАЛИЗАЦИЯ VAO
		// ============================

		GLuint vao[] = { 0,0,0 };
		glGenVertexArrays(3, vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBindVertexArray(vao[0]);
		{
			const auto locationC = glGetAttribLocation(renderProgram.id, "color");
			const auto locationT0 = glGetAttribLocation(renderProgram.id, "t0");
			const auto locationP0 = glGetAttribLocation(renderProgram.id, "p0");
			const auto locationV = glGetAttribLocation(renderProgram.id, "v");
			if (locationT0 == -1 || locationP0 == -1 || locationV == -1 || locationC == -1)
				throw runtime_error("Failed to locate 1render attribute(s)!");

			glVertexAttribPointer(locationC, 3, GL_FLOAT, GL_FALSE, STRIDE, nullptr);
			glVertexAttribPointer(locationT0, 1, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));
			glVertexAttribPointer(locationP0, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(4 * sizeof(GLfloat)));
			glVertexAttribPointer(locationV, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(6 * sizeof(GLfloat)));

			glEnableVertexAttribArray(locationC);
			glEnableVertexAttribArray(locationT0);
			glEnableVertexAttribArray(locationP0);
			glEnableVertexAttribArray(locationV);

		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBindVertexArray(vao[1]);
		{
			const auto locationC = glGetAttribLocation(renderProgram.id, "color");
			const auto locationT0 = glGetAttribLocation(renderProgram.id, "t0");
			const auto locationP0 = glGetAttribLocation(renderProgram.id, "p0");
			const auto locationV = glGetAttribLocation(renderProgram.id, "v");
			if (locationT0 == -1 || locationP0 == -1 || locationV == -1 || locationC == -1)
				throw runtime_error("Failed to locate 2render attribute(s)!");

			glVertexAttribPointer(locationC, 3, GL_FLOAT, GL_FALSE, STRIDE, nullptr);
			glVertexAttribPointer(locationT0, 1, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));
			glVertexAttribPointer(locationP0, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(4 * sizeof(GLfloat)));
			glVertexAttribPointer(locationV, 2, GL_FLOAT, GL_FALSE, STRIDE, reinterpret_cast<GLvoid*>(6 * sizeof(GLfloat)));

			glEnableVertexAttribArray(locationC);
			glEnableVertexAttribArray(locationT0);
			glEnableVertexAttribArray(locationP0);
			glEnableVertexAttribArray(locationV);

		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		/*glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBindVertexArray(vao[2]);
		{
			
			const auto locationCE = glGetAttribLocation(renderEllipseProgram.id, "center");			
				throw runtime_error("Failed to locate 3render attribute(s)!");

			glVertexAttribPointer(locationCE, 1, GL_FLOAT, GL_FALSE, 0, nullptr);		
			glEnableVertexAttribArray(locationCE);

		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);*/

		// ===========================
		// ПРЕДСТАРТОВАЯ ПОДГОТОВКА...
		// ===========================
		glClearColor(0, 0, 0, 1);

		glUseProgram(renderProgram.id);
		glUniform1f(renderPointSizeLocation, POINT_SIZE);
		glUseProgram(0);

		/*glUseProgram(renderEllipseProgram.id);
		glUniform1f(renderEllipsePointSizeLocation, POINT_SIZE);		
		glUniform2f(renderEllipseRadiusLocation, 0.75, 0.4);
		glUniform3f(renderEllipseColorLocation, 0, 0, 255);*/
		CircleProgram circleProgram;
		const GLfloat circleColor[] = { 0.7,1,1 };

		CircleProgram circleProgram2;
		const GLfloat circleColor2[] = { 1,0,0 };

		CircleProgram circleProgram3;

		
		TriangleProgram  triangleProgram;
		const GLfloat triangleColor[] = { 0.5,0.5,0 };
		const GLfloat trianglePoint[] = { 0,0.05,-0.05,-0.05,0.05,-0.05 };

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
			glUseProgram(renderProgram.id);
			glBindVertexArray(vao[0]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);			
			glUniform1f(renderPointTimeLocation, time);
			glDrawArrays(GL_POINTS, 0, N);//GL_LINE_LOOP
			glBindVertexArray(0);
			glUseProgram(0);

			glUseProgram(reflectProgram.id);		
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo[0]);	
			glUniform2f(reflectPoint0Location, -0.25, 0.5);
			glUniform1f(reflectPointTimeLocation, time);
			glDispatchCompute(N, 1, 1);
			glBindVertexArray(0);
			glUseProgram(0);
			
			

			//glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

			// Мы пропускаем привязку VBO к GL_ARRAY_BUFFER,
		   // т. к. у нас только один VBO и он уже привязан к GL_ARRAY_BUFFER
			glUseProgram(renderProgram.id);
			glBindVertexArray(vao[1]);
			glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
			glUniform1f(renderPointTimeLocation, time);
			glDrawArrays(GL_POINTS, 0, N);
			glUseProgram(0);

			glUseProgram(reflectProgram.id);			
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo[1]);			
			glUniform2f(reflectPoint0Location, 0.25, 0.5);
			glUniform1f(reflectPointTimeLocation, time);
			glDispatchCompute(N, 1, 1);
			glUseProgram(0);


			
			circleProgram.Draw(0, 0, 1, 1, circleColor);

			triangleProgram.Draw(trianglePoint, triangleColor);


			circleProgram.DrawingASpecificArea(0, -0.25, 0.25, 0.25, circleColor2,-0.25);
			circleProgram.DrawingASpecificArea(0, -0.25, 0.25, 0.45, circleColor2,-0.25);


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
