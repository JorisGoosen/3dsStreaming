#version 440

in 	vec2 tex;

in  vec4 kleur;
out vec4 col;

uniform sampler2D cam;

void main()
{
	col = texture(cam, tex);;	
}