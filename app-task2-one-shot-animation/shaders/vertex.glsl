#version 430 core

uniform float pointSize;
uniform float pointTime;

in vec2 p;
in float t0;
in vec2 v;
in vec3 rgb;


out vec3 color;

void main() {
gl_Position = vec4(p.x+v.x*(pointTime-t0),p.y+v.y*(pointTime-t0), 0, 1);
gl_PointSize = pointSize;


color = rgb;
}
