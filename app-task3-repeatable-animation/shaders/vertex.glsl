#version 430 core

uniform float pointSize;
uniform float pointTime;

in float t0;
in vec2 p0;
in vec2 v;
in vec3 color;


out vec3 ocolor;
void main() {
    vec2 position = p0 + v * (pointTime - t0);
    gl_PointSize = pointSize;
    gl_Position = vec4(position, 0, 1);
    ocolor=color;
}
