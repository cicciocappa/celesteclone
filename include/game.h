// entities.h
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

// Definizione dell'enumerazione GameState
typedef enum {
    TITLE_SCREEN,
    PLAYING,
    PAUSED,
    GAME_OVER,
} GameState;

// Definizione della struttura Game
typedef struct {
    GameState state; // Campo "state" di tipo GameState
} Game;

#endif