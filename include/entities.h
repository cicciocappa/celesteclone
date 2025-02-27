// entities.h
#ifndef ENTITIES_H
#define ENTITIES_H

#include <stdbool.h>
#include <math.h>
#include <linmath.h>
#include <sprite.h>
 
#define MAX_ENEMIES 50
#define MAX_PROJECTILES 100
#define MAX_STATIC_OBJECTS 8192

typedef struct {
    float x, y;
    float width, height;
} Transform;

typedef struct {
    Transform transform;
    float speed;
    int health;
    bool is_active;
    // Aggiungi altri attributi specifici del player
} Player;

typedef struct {
    Transform transform;
    float speed;
    int damage;
    float patrol_range;
    float patrol_start_x;
    bool is_active;
    // Aggiungi altri attributi specifici del nemico
} Enemy;

typedef struct {
    Transform transform;
    float speed;
    float direction_x;
    float direction_y;
    int damage;
    bool is_active;
    // Aggiungi altri attributi specifici del proiettile
} Projectile;

typedef Sprite EntitaStatica;


// possiamo utilizzare un unico array per gli sprite
// e poi un array diverso per ciascun tipo oggetto
// gli oggetti memorizzano l'indice al relativo sprite nell'array per poterlo modificare quando serve

typedef struct {
   
    Player player;
    
    
    Enemy enemies[MAX_ENEMIES];
    int enemy_count;
    
    Projectile projectiles[MAX_PROJECTILES];
    int projectile_count;

    EntitaStatica decorazioni[MAX_STATIC_OBJECTS];
    size_t count_decorazioni;
   
} GameWorld;



// Funzioni di inizializzazione
void init_game_world(GameWorld* world);
Player* create_player(GameWorld* world, float x, float y);
Enemy* create_enemy(GameWorld* world, float x, float y);
Projectile* create_projectile(GameWorld* world, float x, float y, float dir_x, float dir_y);

// Funzioni di update
void update_player(Player* player, float delta_time);
void update_enemy(Enemy* enemy, float delta_time);
void update_projectile(Projectile* projectile, float delta_time);
void update_game_world(GameWorld* world, float delta_time);

// Funzioni di collisione
bool check_collision(Transform* a, Transform* b);
void handle_collisions(GameWorld* world);

// Funzioni di pulizia
void remove_inactive_entities(GameWorld* world);

#endif