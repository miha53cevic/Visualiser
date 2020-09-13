#version 330
in vec2 position;
in vec2 textureCoords;

out vec2 pass_textureCoords;

uniform mat4 model;
uniform mat4 projection;

void main(void)
{
	gl_Position = projection * model * vec4(position.xy, 0.0, 1.0);
	pass_textureCoords = textureCoords;
}