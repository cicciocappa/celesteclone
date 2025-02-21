#include "sprite.h"
#include <stdlib.h> // For possible memory allocation, if needed

void sprite_init(Sprite *sprite, float x, float y, float width, float height, vec2 uvStart, vec2 uvEnd, float layerIndex, float parX, float parY, float zIndex)
{
    sprite->position[0] = x;
    sprite->position[1] = y;
    sprite->size[0] = width;
    sprite->size[1] = height;
    sprite->rotation = 0.0f;
    vec2_dup(sprite->uvStart, uvStart); // Use linmath's vec2_dup for safe copying
    vec2_dup(sprite->uvEnd, uvEnd);     // Use linmath's vec2_dup for safe copying
    sprite->layerIndex = layerIndex;
    sprite->parallaxFactorX = parX;
    sprite->parallaxFactorY = parY;
    sprite->zIndex = zIndex;
}

void sprite_update(Sprite *sprite, float deltaTime)
{
    // Example: Simple movement and rotation
    sprite->position[0] += 10.0f * deltaTime; // Move 10 pixels/second
    sprite->rotation += 0.5f * deltaTime;     // Rotate 0.5 radians/second
    if (sprite->position[0] > 1200)
    {
        sprite->position[0] = 0;
    }
}

// ... other Sprite-specific function implementations ...