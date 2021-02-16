#version 440

in 	vec2 texl;
in 	vec2 texr;

out vec4 col;

uniform sampler2D cam;

void main()
{
	col = abs(vec4(length(texture(cam, texl)) - length(texture(cam, texr))));	
}