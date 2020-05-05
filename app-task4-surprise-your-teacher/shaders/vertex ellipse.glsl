#version 430 core

uniform float slice;
uniform float cx;
uniform float  cy;
uniform float  rx;
uniform float  ry;
uniform vec2 radius;
uniform vec3 color;
uniform float pointSize;




in float dummy;
const float PI = 3.141592653589;

out vec4 ocolor;

void main() {
    float angle= 2*PI/360*gl_VertexID;
    float x=cx + rx * cos(angle)+dummy;
    float y=cy + ry * sin(angle)+dummy;
    gl_PointSize = pointSize;

    if(slice!=0 && y>slice) ocolor=vec4(color,0);   
    else ocolor=vec4(color,1);  
    gl_Position = vec4(x,y, 0, 1);

    
 
}
