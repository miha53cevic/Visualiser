#version 330
out vec4 Frag_Colour;

uniform vec4 colour;

void main(void)
{
	Frag_Colour = colour;
}