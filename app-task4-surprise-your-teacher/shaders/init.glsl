#version 430 core
// ==============
// ВХОДНЫЕ ДАННЫЕ
// ==============

layout(local_size_x = 1) in;

uniform float pointTime;
uniform vec2 p0;

struct Point {
    vec3 color;
    float t0;  
    vec2 p0;
    vec2 v;  
};

layout(std430, binding = 0) buffer vbo0 {
    Point points1[];
};

//layout(std430, binding = 1) buffer vbo1 {
//    Point points2[];
//};

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


void main() {
    Point point = points1[gl_GlobalInvocationID.x];    
    
    
    point.t0 = pointTime;

    point.p0=p0;

    float angle = atan(point.p0.y,point.p0.x);
    point.v.x =  xorshift()-0.5;
    point.v.y =   xorshift()-0.5;
    point.color.x = xorshift() ;
    point.color.y=xorshift();
    point.color.z=xorshift();
    points1[gl_GlobalInvocationID.x] = point;


//    Point point2 = points2[gl_GlobalInvocationID.x];    
//        
//    point2.t0 = pointTime;
//
//    point2.p0.x =0.5;
//    point2.p0.y = 0.5;
//
//    point2.v.x = xorshift()-0.5;
//    point2.v.y = xorshift()-0.5;
//
//    point2.color.x = xorshift() ;
//    point2.color.y=xorshift();
//    point2.color.z=xorshift();
// 
//    points2[gl_GlobalInvocationID.x] = point2;
}
