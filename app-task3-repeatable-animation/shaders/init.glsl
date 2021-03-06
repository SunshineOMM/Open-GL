#version 430 core
// ==============
// ВХОДНЫЕ ДАННЫЕ
// ==============

layout(local_size_x = 1) in;

uniform float minSpeed;
uniform float maxSpeed;
uniform float pointTime;

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

const float PI = 3.14159;


vec3 unpackColor() 
{
    float f = xorshift();
    vec3 color;
    color.r = floor(f / 65536);
    color.g = floor((f - color.r * 65536) / 256.0);
    color.b = floor(f - color.r * 65536 - color.g * 256.0);
    return color / 256.0;
}


void main() {
    Point point = points[gl_GlobalInvocationID.x];    
    float speed = minSpeed + xorshift() * (maxSpeed - minSpeed);
    
    
    point.t0 = pointTime;
    point.p0.x =-1 + xorshift() * 2;
    point.p0.y = -1 + xorshift() * 2;
    //float angle = 2 * PI * xorshift();
    float angle = atan(point.p0.y,point.p0.x);
    point.v.x = speed * cos(angle);
    point.v.y = speed * sin(angle);
    point.color.x = xorshift() ;
    point.color.y=xorshift();
    point.color.z=xorshift();

    points[gl_GlobalInvocationID.x] = point;
}
