#version 330
in vec2 pass_textureCoords;

out vec4 Frag_Colour;

uniform vec4 colour;

void main(void)
{
	Frag_Colour = colour;
}