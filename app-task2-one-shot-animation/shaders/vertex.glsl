#version 430 core

uniform float pointSize;
uniform float pointTime;

in vec2 p;
in float s;
in vec3 rgb;


out vec3 color;

void main() {
float k=p.y/p.y;
float cosP=sqrt(1/(1-pow(k,2)));
float sinP=sqrt(1/(1-1/pow(k,2)));
gl_Position = vec4(p.x+cosP*pointTime*s,p.y+sinP*pointTime*s, 0, 1);
gl_PointSize=pointSize;
color = rgb;
}