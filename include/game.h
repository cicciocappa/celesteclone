// entities.h
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Definizione dell'enumerazione GameState
typedef enum
{
    TITLE_SCREEN,
    PLAYING,
    PAUSED,
    GAME_OVER,
} GameState;

// Definizione della struttura Game
typedef struct
{
    GameState state; // Campo "state" di tipo GameState
    int screenWidth;
    int screenHeight;
    GLFWwindow *window;
    bool running;
    vec2 camera_pos;
} Game;

extern Game game;

bool init_game();
void update(float deltaTime);
void render();
void cleanup();

#endif