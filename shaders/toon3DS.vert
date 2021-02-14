#version 440

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTex;

out vec2 tex;
out vec4 kleur;

void main(){
	gl_Position = vec4(vPos, 0.0, 1.0);
  	tex = vTex;

	kleur = vec4(tex, 0,  1);
}