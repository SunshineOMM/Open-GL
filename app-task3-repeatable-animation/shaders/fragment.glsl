#version 430 core

in vec3 ocolor;

out vec4 outColor;

void main() {
    outColor = vec4(ocolor, 1);
}
