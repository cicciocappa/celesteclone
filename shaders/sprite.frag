#version 430 core

in vec2 texCoord;
in flat int spriteID;

out vec4 FragColor;

// Define the sprite data structure
struct SpriteData {
    vec2 uvStart;
    vec2 uvEnd;
    float layerIndex;
    float zIndex;
    vec2 position;
    vec2 size;
    float rotation;
    float parallaxFactorX;
    float parallaxFactorY;
    vec3 color;
};

layout (std430, binding = 0) buffer SpriteBuffer {
    SpriteData sprites[];
};

// Texture array sampler
uniform sampler2DArray textureArray;

void main() {
    // Get the sprite data
    SpriteData sprite = sprites[spriteID];
    
    // Interpolate between uvStart and uvEnd using texCoord
    vec2 finalUV = mix(sprite.uvStart, sprite.uvEnd, texCoord);
    
    // Sample the texture using the final UV and layer index
    vec4 texColor = texture(textureArray, vec3(finalUV, sprite.layerIndex));

    if (texColor.a < 0.5) // o qualsiasi altra soglia
    discard;
    
    // Output the final color
    FragColor = texColor * vec4(sprite.color, 1.0);
}