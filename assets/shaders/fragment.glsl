#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D u_Texture;

void main()
{
	FragColor = texture(u_Texture, TexCoord);//vec4(TexCoord.x * 16.0, TexCoord.y * 16.0, 0.0, 1.0);
}