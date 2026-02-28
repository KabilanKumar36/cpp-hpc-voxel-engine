#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out float Visibility; // For Fog Calculation
out vec3 VoxelUVW; // 3D Texture Coordinate

uniform mat4 uViewProjection; // Camera View * Projection Matrix

uniform vec2 u_ChunkOffset;

// Fog Settings
// Lower density = thicker fog further away
// Higher gradient = sharper transition
const float density = 0.015;
const float gradient = 1.5;

const float CHUNK_SIZE = 16;
const float CHUNK_HEIGHT = 16;

void main()
{
    // 1. Calculate Clip Space Position
	gl_Position = uViewProjection * vec4(aPos, 1.0);
    
    // 2. Pass Texture Coordinates
	TexCoord = aTexCoord;
	
	// Calculate normalized 3D coordinate (0.0 to 1.0) mapping the 16x16x16 chunk
    float fLocalX = (aPos.x - u_ChunkOffset.x) / CHUNK_SIZE;
    float fLocalY = aPos.y / CHUNK_HEIGHT;
    float fLocalZ = (aPos.z - u_ChunkOffset.y) / CHUNK_SIZE;
    VoxelUVW = vec3(fLocalX, fLocalY, fLocalZ);
    
    // 3. Calculate Fog Visibility based on distance from camera
    // gl_Position.z is the depth/distance relative to the camera
    //float distance = length(gl_Position.xyz);
    float distance = gl_Position.w;
    Visibility = exp(-pow((distance * density), gradient));
    
    // Clamp to ensure valid range [0, 1]
    Visibility = clamp(Visibility, 0.0, 1.0);
}