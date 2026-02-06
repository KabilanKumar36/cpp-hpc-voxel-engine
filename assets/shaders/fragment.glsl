#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D u_Texture;

void main()
{
	//Red Flash Test
	//FragColor = vec4(1.0, 0.0, 0.0, 1.0);

	//RGB Test
	//FragColor = vec4(TexCoord.x * 16.0, TexCoord.y * 16.0, 0.0, 1.0);
	
	//texture
	FragColor = texture(u_Texture, TexCoord);
}