#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "renderer.h"
#include "sprite.h"
#include "game.h"

int main()
{

    // Add at the beginning of main()
    srand(time(NULL)); // Seed the random number generator

    // --- GLFW Initialization --- (your boilerplate)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /*
    int screenWidth = 800;
    int screenHeight = 600;
    GLFWwindow *window = glfwCreateWindow(screenWidth, screenHeight, "Sprite Rendering", NULL, NULL);
    */
    // Ottieni il monitor primario
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor)
    {

        fprintf(stderr, "Impossibile ottenere il monitor primario\n");
        glfwTerminate();
        return -1;
    }

    // Ottieni la modalità video corrente del monitor
    const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
    if (!mode)
    {
        fprintf(stderr, "Impossibile ottenere la modalità video del monitor\n");

        glfwTerminate();
        return -1;
    }
    int screenWidth = mode->width;
    int screenHeight = mode->height;
    // Imposta gli attributi della finestra (opzionale)
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // Crea la finestra in modalità fullscreen
    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Finestra Fullscreen", primaryMonitor, NULL);

    if (window == NULL)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    glViewport(0, 0, screenWidth, screenHeight);

    // --- Renderer and Sprite Setup ---
    Renderer renderer;
    const size_t MAX_SPRITES = 1000;
    if (!renderer_init(&renderer, MAX_SPRITES, screenWidth, screenHeight))
    {
        fprintf(stderr, "Failed to initialize renderer\n");
        return -1;
    }

    Sprite *sprites = (Sprite *)malloc(MAX_SPRITES * sizeof(Sprite));
    size_t numSprites = 60;

    for (size_t i = 0; i < numSprites; ++i)
    {
        /*
        // Then in your sprite initialization loop
        vec4 color = {
            (float)rand() / RAND_MAX, // Random red component
            (float)rand() / RAND_MAX, // Random green component
            (float)rand() / RAND_MAX, // Random blue component
            1.0f                      // Full opacity
        };
        */
        size_t px = i % 10;
        size_t py = i / 10;
        vec2 uvStart = {0.0f, 0.0f};
        vec2 uvEnd = {1.0f / 8.0f, 1.0f / 8.0f};
        float layerIndex = (float)(rand() % 4);
        sprite_init(&sprites[i], 20.0f + px * 40.0f, 20.0f + py * 40.0f, 32.0f, 32.0f, uvStart, uvEnd, layerIndex);
    }

    // --- Main Loop ---
    while (!glfwWindowShouldClose(window))
    {
        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, 1);

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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    renderer_cleanup(&renderer);
    free(sprites);
    glfwTerminate();
    return 0;
}