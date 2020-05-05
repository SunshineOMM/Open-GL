#version 430 core

in vec2 p;


uniform vec3 color;
uniform float pointSize;

out vec4 ocolor;
void main() {
    gl_PointSize = pointSize;
    gl_Position = vec4(p, 0, 1);
    ocolor=vec4(color,1);
}
