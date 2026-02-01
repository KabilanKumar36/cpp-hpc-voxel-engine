#version 460 core
out vec4 FragColor;

uniform vec3 colorVal;

void main()
{
	FragColor = vec4(colorVal, 1.0);
}