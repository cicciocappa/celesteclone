#include "game.h"
#include <time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

Game game;

// Funzione di callback per gli eventi di tastiera
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        printf("Tasto SPAZIO premuto\n");
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        //glfwSetWindowShouldClose(window, GLFW_TRUE); // Chiude la finestra
        game.running = false;
    }
}

bool init_game() {
    game.running = true;
    srand(time(NULL)); // Seed the random number generator
    // --- GLFW Initialization --- (your boilerplate)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor)
    {

        fprintf(stderr, "Impossibile ottenere il monitor primario\n");
        glfwTerminate();
        return false;
    }

    // Ottieni la modalità video corrente del monitor
    const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
    if (!mode)
    {
        fprintf(stderr, "Impossibile ottenere la modalità video del monitor\n");

        glfwTerminate();
        return false;
    }
    game.screenWidth = mode->width;
    game.screenHeight = mode->height;
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
        return false;
    }
    game.window = window;
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    //printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    //printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    glViewport(0, 0, game.screenWidth, game.screenHeight);
    return true;
}

void update(float deltaTime){

}

void render(){
    
}

void cleanup(){
    glfwTerminate();
}