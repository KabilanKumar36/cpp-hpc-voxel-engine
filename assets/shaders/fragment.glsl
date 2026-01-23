#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);//vec4(1.0, 0.5, 0.2, 1.0);
}