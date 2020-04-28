#version 430 core

uniform float pointSize;
uniform float t;

in float t0;
in vec2 p0;
in vec2 v;

void main() {
    vec2 position = p0 + v * (t - t0);
    gl_PointSize = pointSize;
    gl_Position = vec4(position, 0, 1);
}
