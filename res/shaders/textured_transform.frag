#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;

void main()
{
	//texture is a function
	FragColor = texture(texture1, TexCoord);
}