#version 430 core

uniform float pointSize;
uniform float pointTime;

in vec2 p;
in float s;
in vec3 rgb;


out vec3 color;

void main() {
if(p.x>0){
	if(p.y>0)	gl_Position=vec4(p.x+s*pointTime,p.y+s*pointTime,0,1);
	if(p.y<0)	gl_Position=vec4(p.x+s*pointTime,p.y-s*pointTime,0,1);
	if(p.y==0)	gl_Position=vec4(p.x+s*pointTime,p.y,0,1);
}
else{
	
	if(p.y>0){
		if(p.x==0) 	gl_Position=vec4(p.x,p.y+s*pointTime,0,1);
		else	gl_Position=vec4(p.x-s*pointTime,p.y+s*pointTime,0,1);
	}

	if(p.y<0){
		if(p.x==0) gl_Position=vec4(p.x,p.y-s*pointTime,0,1);
		else	gl_Position=vec4(p.x-s*pointTime,p.y-s*pointTime,0,1);
	}
	if(p.y==0)	gl_Position=vec4(p.x-s*pointTime,p.y,0,1);
}
gl_PointSize = pointSize;


color = rgb;
}
//gl_Position = vec4(p.x+cosP*pointTime*s,p.y+sinP*pointTime*s, 0, 1);