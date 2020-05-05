#version 430 core

// ==============
// ВХОДНЫЕ ДАННЫЕ
// ==============

layout(local_size_x = 1) in;

uniform float pointTime;
uniform float minSpeed;
uniform float maxSpeed;

struct Point {

    vec3 color;
    float t0;   
    vec2 p0;
    vec2 v;  
};

layout(std430, binding = 0) buffer ssbo {
    Point points[];
};


// ===============================
// ГЕНЕРАТОР ПСЕВДОСЛУЧАЙНЫХ ЧИСЕЛ
// ===============================


// "Перемешивает" два значения в одно
uint xxhash(uint x, uint y) {
    x = x * 3266489917 + 374761393;
    x = (x << 17) | (x >> 15);
    x += y * 3266489917;
    x *= 668265263;
    x ^= x >> 15;
    x *= 2246822519;
    x ^= x >> 13;
    x *= 3266489917;
    x ^= x >> 16;
    return x;
}

// Внутреннее состояние xorshift
uint xorshiftState = xxhash(floatBitsToUint(pointTime), gl_GlobalInvocationID.x);

// Xorshift - возвращает псевдослучайное значение в диапазоне [0, 1]
float xorshift() {
    uint x = xorshiftState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    xorshiftState = x;

    // Приведение к диапазону [0, 1]
    uint m = 0x00ffffff;
    return (x & m) * (1.0f / 0x1000000);
}


// ================
// ОСНОВНАЯ ФУНКЦИЯ
// ================

void main() {
    Point point = points[gl_GlobalInvocationID.x];

    // Вычисляем текущие координаты точки
    vec2 p = point.p0 + point.v * (pointTime - point.t0);

    // Отражаем вектор скорости, если точка достигла края окна
    if (p.x <= -1 && point.v.x < 0 || p.x >= 1 && point.v.x > 0 || p.y <= -1 && point.v.y < 0 || p.y >= 1 && point.v.y > 0) {         
        point.t0 = pointTime;
        point.p0 = vec2(xorshift()-0.5,xorshift()-0.5);
        float angle = atan(point.p0.y,point.p0.x);
        point.v.x = cos(angle);
        point.v.y = sin(angle);
        point.color.x = xorshift() ;
        point.color.y=xorshift();
        point.color.z=xorshift();
        points[gl_GlobalInvocationID.x] = point;
    }
}
