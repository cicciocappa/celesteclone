#include "sprite.h"
#include <stdlib.h> // For possible memory allocation, if needed

void sprite_init(Sprite* sprite, float x, float y, float width, float height, vec4 color) {
    sprite->position[0] = x;
    sprite->position[1] = y;
    sprite->size[0] = width;
    sprite->size[1] = height;
    sprite->rotation = 0.0f;
    vec4_dup(sprite->color, color); // Use linmath's vec4_dup for safe copying
}

void sprite_update(Sprite* sprite, float deltaTime) {
    // Example: Simple movement and rotation
    sprite->position[0] += 10.0f * deltaTime;  // Move 10 pixels/second
    sprite->rotation += 0.5f * deltaTime; // Rotate 0.5 radians/second
    if(sprite->position[0] > 800) {
        sprite->position[0] = 0;
    }
}

// ... other Sprite-specific function implementations ...