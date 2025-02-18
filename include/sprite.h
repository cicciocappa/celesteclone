#ifndef SPRITE_H
#define SPRITE_H

#include <linmath.h>

typedef struct {
    vec2 uvStart;       // 8 bytes, offset 0
    vec2 uvEnd;         // 8 bytes, offset 8
    float layerIndex;   // 4 bytes, offset 16 (ora è un float per compatibilità con GLSL)
    float padding;      // 4 bytes, offset 20
    vec2 position;      // 8 bytes, offset 24
    vec2 size;          // 8 bytes, offset 32
    float rotation;     // 4 bytes, offset 36
    float padding2[3];  // 12 bytes padding, offset 40
} Sprite; // Total: 52 bytes

// Function declarations related to Sprite *data* manipulation
void sprite_init(Sprite* sprite, float x, float y, float width, float height, vec2 uvStart, vec2 uvEnd, float layerIndex); // Example
void sprite_update(Sprite* sprite, float deltaTime); // Example: Update position, rotation, etc.
// ... other Sprite-specific functions ...

#endif // SPRITE_H