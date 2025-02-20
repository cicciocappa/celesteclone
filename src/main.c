#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "renderer.h"
#include "sprite.h"
#include "game.h"

int main()
{

    // Add at the beginning of main()
   

    if (!init_game()){
        fprintf(stderr, "Failed to initialize game\n");
        return -1;
    }

    double lastTime = glfwGetTime();
    double deltaTime = 0.0;


    // Game loop
    while (game.running) {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        update(deltaTime);
        // Il tuo codice di rendering qui
        render();
        glfwSwapBuffers(game.window);
        glfwPollEvents();
    }

    cleanup();
    return 0;
    /*
    Renderer renderer;
    const size_t MAX_SPRITES = 1000;
    if (!renderer_init(&renderer, MAX_SPRITES, game.screenWidth, game.screenHeight))
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        return -1;
    }

    Sprite *sprites = (Sprite *)malloc(MAX_SPRITES * sizeof(Sprite));
    size_t numSprites = 360;

    for (size_t i = 0; i < numSprites; ++i)
    {
       
        size_t px = i % 30;
        size_t py = i / 30;
        vec2 uvStart = {0.0f, 0.0f};
        vec2 uvEnd = {1.0f / 8.0f, 1.0f / 8.0f};
        float layerIndex = (float)(rand() % 4);
        sprite_init(&sprites[i], 20.0f + px * 40.0f, 20.0f + py * 40.0f, 32.0f, 32.0f, uvStart, uvEnd, layerIndex);
    }

    // --- Main Loop ---
    while (!glfwWindowShouldClose(game.window))
    {
        // Input
        if (glfwGetKey(game.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(game.window, 1);

        // Update
        float deltaTime = (float)glfwGetTime(); // Simple deltatime. You can improve this
        glfwSetTime(0);                         // Resetting time

        for (size_t i = 0; i < numSprites; i++)
        {
            sprite_update(&sprites[i], deltaTime);
        }

        // Render
        renderer_begin_frame(&renderer);
        renderer_draw_sprites(&renderer, sprites, numSprites);
        renderer_end_frame(&renderer);

        glfwSwapBuffers(game.window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    renderer_cleanup(&renderer);
    free(sprites);
    glfwTerminate();
    return 0;


    */

     /*
        // Then in your sprite initialization loop
        vec4 color = {
            (float)rand() / RAND_MAX, // Random red component
            (float)rand() / RAND_MAX, // Random green component
            (float)rand() / RAND_MAX, // Random blue component
            1.0f                      // Full opacity
        };
        */

}