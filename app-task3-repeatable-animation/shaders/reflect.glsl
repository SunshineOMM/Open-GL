#version 430 core

// ==============
// ВХОДНЫЕ ДАННЫЕ
// ==============

layout(local_size_x = 1) in;

uniform float pointTime;

struct Point {
    float t0;
    vec2 p0;
    vec2 v;
    //vec3 color;
};

layout(std430, binding = 0) buffer ssbo {
    Point points[];
};

// ================
// ОСНОВНАЯ ФУНКЦИЯ
// ================

void main() {
    Point point = points[gl_GlobalInvocationID.x];

    // Вычисляем текущие координаты точки
    vec2 p = point.p0 + point.v * (pointTime - point.t0);
    bool changed = false;

    // Отражаем вектор скорости, если точка достигла края окна
    if (p.x <= -1 && point.v.x < 0 || p.x >= 1 && point.v.x > 0) {
        point.v.x = -point.v.x;
        changed = true;
    }
    if (p.y <= -1 && point.v.y < 0 || p.y >= 1 && point.v.y > 0) {
        point.v.y = -point.v.y;
        changed = true;
    }

    if (changed) {
        point.t0 = pointTime;
        point.p0 = p;
        points[gl_GlobalInvocationID.x] = point;
    }
}
