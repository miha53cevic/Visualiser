#version 330
out vec4 frag_colour;

uniform vec4 colour;

void main(void)
{
	frag_colour = colour;
}