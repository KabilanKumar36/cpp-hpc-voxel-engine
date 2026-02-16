#version 450 core

// Attributes from Chunk::updateBuffers()
// layout(location = 0) in vec3 aPos;     // Position
// layout(location = 1) in vec2 aTexCoord;// Texture Coordinate

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out float Visibility; // For Fog Calculation

uniform mat4 uViewProjection; // Camera View * Projection Matrix

// Fog Settings
// Lower density = thicker fog further away
// Higher gradient = sharper transition
const float density = 0.007;
const float gradient = 1.5;

void main()
{
    // 1. Calculate Clip Space Position
	gl_Position = uViewProjection * vec4(aPos, 1.0);
    
    // 2. Pass Texture Coordinates
	TexCoord = aTexCoord;

    // 3. Calculate Fog Visibility based on distance from camera
    // gl_Position.z is the depth/distance relative to the camera
    float distance = length(gl_Position.xyz);
    Visibility = exp(-pow((distance * density), gradient));
    
    // Clamp to ensure valid range [0, 1]
    Visibility = clamp(Visibility, 0.0, 1.0);
}