#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h> // Include OpenGL headers here
#include <GLFW/glfw3.h>
#include "sprite.h"     // Include the Sprite struct definition
#include <linmath.h>

typedef struct {
    GLuint quadVAO;
    GLuint instanceSSBO;
    GLuint shaderProgram;
    mat4x4 projection;
    size_t maxSprites;
} Renderer;

// Function declarations related to rendering
int renderer_init(Renderer* renderer, size_t maxSprites, int screenWidth, int screenHeight);
void renderer_begin_frame(Renderer* renderer); //Might be used to setup things needed at the beginning of each frame
void renderer_draw_sprites(Renderer* renderer, Sprite* sprites, size_t numSprites);
void renderer_end_frame(Renderer* renderer);   //Might be used to execute drawing commands
void renderer_cleanup(Renderer* renderer);

#endif // RENDERER_H