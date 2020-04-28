#version 430 core

uniform float pointSize;

in vec2 p; 
in vec3 rgb;

out vec3 color;

void main() { 
gl_Position = vec4(p, 0, 1);
gl_PointSize=pointSize;
color = rgb;
}