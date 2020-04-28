#include <core.h>
#include <fstream>
#include <sstream>   
#include <stdexcept> 
#include <vector> 

using namespace std;

// Вспомогательный метод для компиляции одного шейдера.
// Принимает путь к файлу с исходным кодом шейдера и тип создаваемого шейдера.
// Возвращает идентификатор созданного шейдера.
// Генерирует исключение в случае какой-либо ошибки.
static GLuint CompileShader(const string& path, const GLenum shaderType) {
    // TODO: См. раздел 1.4 в методичке.
    // Шаг 1. Открыть файл, считать все его строки, закрыть файл.
    string sourceCode;
    {
        ifstream fs(path);
        stringstream ss;
        ss << fs.rdbuf();   //ЗАПОМНИТЬ!!!
        if (fs.fail()) {
            stringstream errorMessage;
            errorMessage << "Failed to load shader from file!" << endl;
            errorMessage << "Shader source file: \"" << path << "\".";
            throw runtime_error(errorMessage.str().c_str());
        }
        sourceCode = ss.str();
    }
    // Шаг 2. Создать объект шейдера.
    auto id = glCreateShader(shaderType);
    if (id == 0) {
        stringstream errorMessage;
        errorMessage << "Failed to create shader with type " << shaderType << "!" << endl;
        errorMessage << "Shader source file: \"" << path << "\".";
        throw runtime_error(errorMessage.str().c_str());
    }
    // Шаг 3. Привязать к созданному шейдеру считанные из файла строки исходного кода.
    const auto shaderPtr = sourceCode.c_str();
    glShaderSource(id, 1, &shaderPtr, nullptr);
    // Шаг 4. Скомпилировать шейдер.
    glCompileShader(id);
    // Шаг 5. Убедиться в успешности компиляции.
    GLint success = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);// запись данных в success
    if (success == GL_FALSE) {// случай, если не корректно скомпилировался шейдер
        GLint logLength = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);// получение длины лога
        vector<GLchar> log(logLength);// создание контейнера под лог
        glGetShaderInfoLog(id, logLength, nullptr, log.data());// запись в контейнер лога сам лог
        glDeleteShader(id);// удаление шейдера
        stringstream errorMessage;// создание и формирование текста исключения
        errorMessage << "Failed to compile shader!" << endl;
        errorMessage << "Shader source file: \"" << path << "\"." << endl;
        if (!log.empty()) {
            errorMessage << log.data();// печать в поток ошибок лога
        }
        throw runtime_error(errorMessage.str().c_str());// проброс исключения
    }
    // Шаг 6. В случае успеха вернуть идентификатор скомпилированного шейдера.
    // Аккуратно проверяйте успешность выполнения каждого шага.
    // После шага 2 в случае любой ошибки не забудьте удалить объект созданного шейдера перед тем, как передать управление внешнему коду.
    // В случае неуспеха компиляции запросить лог и сгенерировать исключение с этим логом.
    return id;
}
    
    

    


// Конструктор.
// Принимает произвольный набор пар вида ("Путь к файлу с исходным кодом шейдера", "Тип шейдера").
// Компилирует все указанные шейдеры и линкует их в шейдерную программу.
// Генерирует исключение в случае какой-либо ошибки.
ShaderProgram::ShaderProgram(initializer_list<pair<string, GLenum>> shaders) : id(glCreateProgram()) {          //ЗАПОМНИТЬ!!! в параметрах определение списка инициализации
    // TODO: См. раздел 1.4 в методичке. Не забудьте поправить список инициализации!
    // Шаг 1. Создать объект программы.
    if (id == 0)
        throw runtime_error("Failed to create shader program!");
    // Шаг 2. Пройтись по всем парам и для каждой пары вызвать CompileShader и присоединить скомпилированный шейдер к программе.
    vector<GLuint> compiledShaders;
    compiledShaders.reserve(shaders.size());    //ЗАПОМНИТЬ!!! заранее резервирование памяти под данные
    try {
        for (auto& shader : shaders) {
            const auto shaderId = CompileShader(shader.first, shader.second); //компиляция
            compiledShaders.push_back(shaderId);
            glAttachShader(id, shaderId);
        }
        // Шаг 3. Слинковать программу.
        glLinkProgram(id);
        // Независимо от успеха/неуспеха операции в конце удалить объекты всех созданных шейдеров.
        // В случае неуспеха удалить объект созданной программы.
        // В случае неуспеха линковки запросить лог и сгенерировать исключение с этим логом.
        GLint success = GL_FALSE;
        glGetProgramiv(id, GL_LINK_STATUS, &success); // аналогично как и с компиляцией шейдера
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

        for (auto shaderId : compiledShaders)   // удаление шейдеров
            glDeleteShader(shaderId);
    }
    catch (...) {
        for (auto shaderId : compiledShaders)
            glDeleteShader(shaderId);
        glDeleteProgram(id);
        throw;
    }

}

// Деструктор.
// Удаляет объект шейдерной программы.
ShaderProgram::~ShaderProgram() {
    glDeleteShader(id);
}
