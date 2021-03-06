#version 440

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTex;

out vec2 texl;
out vec2 texr;

void main(){
	gl_Position = vec4(vPos, 0.0, 1.0);
  	texl = vTex  * vec2(1, -1);	
	texr = texl - vec2(-0.016, 0.022); //manually calibrated ;)
}