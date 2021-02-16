#version 440

in 	vec2 texl;
in 	vec2 texr;

out vec4 col;

layout(binding=0) uniform sampler2D caml;
layout(binding=1) uniform sampler2D camr;

void main()
{
	float v = length(texture(caml, texl)) - length(texture(camr, texr));
	col = vec4(v, -v, 0, 1);
}