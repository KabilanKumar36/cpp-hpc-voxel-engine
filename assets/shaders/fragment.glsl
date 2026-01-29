#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexCoord);//vec4(TexCoord.x * 16.0, TexCoord.y * 16.0, 0.0, 1.0);
}