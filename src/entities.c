
// entities.c
#include "entities.h"
#include <string.h>
#include <stdlib.h>

void init_game_world(GameWorld *world)
{
    memset(world, 0, sizeof(GameWorld));
    for (size_t i = 0; i < 200; ++i)
    {
        size_t px = i % 30;
        size_t py = i / 30;
        vec2 uvStart = {0.0f, 0.0f};
        vec2 uvEnd = {1.0f / 8.0f, 1.0f / 8.0f};
        float layerIndex = (float)(rand() % 4);
        float parallax = 1.0f- (float)(rand()%2)*0.5f;
        sprite_init(&world->decorazioni[i], 20.0f + px * 40.0f, 20.0f + py * 40.0f, 32.0f, 32.0f, uvStart, uvEnd, layerIndex, parallax, parallax, parallax);
    }
    (*world).count_decorazioni = 200;
}

Player *create_player(GameWorld *world, float x, float y)
{

    Player *player = &world->player;

    player->transform.x = x;
    player->transform.y = y;
    player->transform.width = 32; // Dimensioni esempio
    player->transform.height = 32;
    player->speed = 200.0f; // Pixel per secondo
    player->health = 100;
    player->is_active = true;

    return player;
}

Enemy *create_enemy(GameWorld *world, float x, float y)
{
    if (world->enemy_count >= MAX_ENEMIES)
        return NULL;

    Enemy *enemy = &world->enemies[world->enemy_count++];
    enemy->transform.x = x;
    enemy->transform.y = y;
    enemy->transform.width = 32;
    enemy->transform.height = 32;
    enemy->speed = 100.0f;
    enemy->damage = 10;
    enemy->patrol_range = 100.0f;
    enemy->patrol_start_x = x;
    enemy->is_active = true;

    return enemy;
}

Projectile *create_projectile(GameWorld *world, float x, float y, float dir_x, float dir_y)
{
    if (world->projectile_count >= MAX_PROJECTILES)
        return NULL;

    Projectile *projectile = &world->projectiles[world->projectile_count++];
    projectile->transform.x = x;
    projectile->transform.y = y;
    projectile->transform.width = 8;
    projectile->transform.height = 8;
    projectile->speed = 300.0f;
    projectile->direction_x = dir_x;
    projectile->direction_y = dir_y;
    projectile->damage = 20;
    projectile->is_active = true;

    return projectile;
}

void update_player(Player *player, float delta_time)
{
    if (!player->is_active)
        return;
    // Implementa la logica di movimento del player
    // Ad esempio: input da tastiera, limiti dello schermo, ecc.
}

void update_enemy(Enemy *enemy, float delta_time)
{
    if (!enemy->is_active)
        return;

    // Esempio di movimento di pattugliamento
    float patrol_distance = enemy->transform.x - enemy->patrol_start_x;
    if (fabsf(patrol_distance) > enemy->patrol_range)
    {
        enemy->speed = -enemy->speed; // Inverti direzione
    }
    enemy->transform.x += enemy->speed * delta_time;
}

void update_projectile(Projectile *projectile, float delta_time)
{
    if (!projectile->is_active)
        return;

    projectile->transform.x += projectile->direction_x * projectile->speed * delta_time;
    projectile->transform.y += projectile->direction_y * projectile->speed * delta_time;

    // Disattiva il proiettile se esce dallo schermo
    // Sostituisci SCREEN_WIDTH e SCREEN_HEIGHT con i valori effettivi
    if (projectile->transform.x < 0 || projectile->transform.x > 800 ||
        projectile->transform.y < 0 || projectile->transform.y > 600)
    {
        projectile->is_active = false;
    }
}

void update_game_world(GameWorld *world, float delta_time)
{
    // Aggiorna tutte le entità

    update_player(&world->player, delta_time);

    for (int i = 0; i < world->enemy_count; i++)
    {
        update_enemy(&world->enemies[i], delta_time);
    }

    for (int i = 0; i < world->projectile_count; i++)
    {
        update_projectile(&world->projectiles[i], delta_time);
    }

    // Gestisci le collisioni
    handle_collisions(world);

    // Rimuovi le entità inattive
    remove_inactive_entities(world);
}

bool check_collision(Transform *a, Transform *b)
{
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

void handle_collisions(GameWorld *world)
{
    // Controlla collisioni proiettili-nemici
    for (int i = 0; i < world->projectile_count; i++)
    {
        if (!world->projectiles[i].is_active)
            continue;

        for (int j = 0; j < world->enemy_count; j++)
        {
            if (!world->enemies[j].is_active)
                continue;

            if (check_collision(&world->projectiles[i].transform,
                                &world->enemies[j].transform))
            {
                // Gestisci la collisione
                world->projectiles[i].is_active = false;
                world->enemies[j].is_active = false;
                break;
            }
        }
    }

    // Aggiungi altre verifiche di collisione secondo necessità
}

void remove_inactive_entities(GameWorld *world)
{
    // Rimuovi nemici inattivi
    int active_enemies = 0;
    for (int i = 0; i < world->enemy_count; i++)
    {
        if (world->enemies[i].is_active)
        {
            if (i != active_enemies)
            {
                world->enemies[active_enemies] = world->enemies[i];
            }
            active_enemies++;
        }
    }
    world->enemy_count = active_enemies;

    // Rimuovi proiettili inattivi
    int active_projectiles = 0;
    for (int i = 0; i < world->projectile_count; i++)
    {
        if (world->projectiles[i].is_active)
        {
            if (i != active_projectiles)
            {
                world->projectiles[active_projectiles] = world->projectiles[i];
            }
            active_projectiles++;
        }
    }
    world->projectile_count = active_projectiles;
}