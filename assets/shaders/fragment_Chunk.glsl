#version 450 core
out vec4 FragColor;

in vec2 TexCoord;
in float Visibility; // 1.0 = Clear, 0.0 = Full Fog
in vec3 VoxelUVW; // From Vertex Shader

uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler3D u_ThermalTexture; // The 3D temperature volume (Unit 1)

void main()
{
    // 1. Sample Texture
    vec4 texColor = texture(u_Texture, TexCoord);
    
    // Alpha discard for transparency (leaves, glass, etc.)
    if(texColor.a < 0.1)
        discard;

    /* // 2. Apply Fog
    // Mix between the Texture Color and the Sky Color (e.g., Light Blue)
    vec3 skyColor = vec3(0.2, 0.3, 0.2); // Matches a typical clear sky
    vec3 finalColor = mix(skyColor, texColor.rgb, Visibility);*/
    
    // Read raw temperature value
    float temp = texture(u_ThermalTexture, VoxelUVW).r;

	// Calculate Heat Glow Color (Mapping 0.0 -> 5000.0)
    vec3 glowColor = vec3(0.0);
    if (temp > 1.0) { 
        float normalizedTemp = clamp(temp / 5000.0, 0.0, 1.0);
        vec3 hotColor = vec3(1.0, 0.2, 0.0);      
        vec3 superHotColor = vec3(1.0, 1.0, 0.8); 
        
        if (normalizedTemp < 0.5) {
            glowColor = mix(vec3(0.0), hotColor, normalizedTemp * 2.0);
        } else {
            glowColor = mix(hotColor, superHotColor, (normalizedTemp - 0.5) * 2.0);
        }
    }

    // Additive blend the base color and the heat
    vec3 glowingBase = texColor.rgb + glowColor;

    vec3 skyColor = vec3(0.2, 0.3, 0.2);

    // 2. Apply Fog and temperature
    vec3 finalColor = mix(skyColor, glowingBase, Visibility);

    FragColor = vec4(finalColor, texColor.a);
}