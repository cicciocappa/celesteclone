#ifndef SPRITE_H
#define SPRITE_H

#include <linmath.h>

typedef struct {
    vec4 color;       // 16 bytes, offset 0
    vec2 position;    // 8 bytes, offset 16
    vec2 size;        // 8 bytes, offset 24
    float rotation;   // 4 bytes, offset 32
    float padding[3]; // 12 bytes padding, offset 36 
                      // (to make vec4 start at multiple of 16)
   
} Sprite;            // Total: 48 bytes

// Function declarations related to Sprite *data* manipulation
void sprite_init(Sprite* sprite, float x, float y, float width, float height, vec4 color); // Example
void sprite_update(Sprite* sprite, float deltaTime); // Example: Update position, rotation, etc.
// ... other Sprite-specific functions ...

#endif // SPRITE_H