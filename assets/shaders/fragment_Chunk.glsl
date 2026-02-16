#version 450 core
out vec4 FragColor;

in vec2 TexCoord;
in float Visibility; // 1.0 = Clear, 0.0 = Full Fog

uniform sampler2D u_Texture;

void main()
{
    // 1. Sample Texture
    vec4 texColor = texture(u_Texture, TexCoord);
    
    // Alpha discard for transparency (leaves, glass, etc.)
    if(texColor.a < 0.1)
        discard;

    // 2. Apply Fog
    // Mix between the Texture Color and the Sky Color (e.g., Light Blue)
    vec3 skyColor = vec3(0.2, 0.3, 0.2); // Matches a typical clear sky
    vec3 finalColor = mix(skyColor, texColor.rgb, Visibility);

    FragColor = vec4(finalColor, texColor.a);
	
    // Debug: Uncomment to visualize UVs
    // FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
}