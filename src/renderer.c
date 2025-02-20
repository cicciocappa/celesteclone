

#include "renderer.h"
#include "game.h"
#include <stdio.h> //for error messages
#include <stdlib.h>
#include <string.h> // For strdup
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLint cameraPosLoc;

// Helper function to read the entire contents of a file into a string
char *read_file_to_string(const char *filename)
{
    FILE *file = fopen(filename, "rb"); // Open in binary mode for cross-platform compatibility
    if (!file)
    {
        perror("Failed to open file"); // Use perror for system error messages
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1); // +1 for null terminator
    if (!buffer)
    {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size)
    {
        fprintf(stderr, "Error reading file: %s\n", filename); // More specific error
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0'; // Null-terminate the string
    fclose(file);

    return buffer;
}

// --- (Your shader compilation functions from the previous example) ---
GLuint compile_shader(GLenum type, const char *source);
GLuint create_program(GLuint vertexShader, GLuint fragmentShader);

// --- (Your shader compilation functions - MODIFIED to take char* instead of #include) ---
GLuint compile_shader(GLenum type, const char *source)
{
    // ... (Exactly as before, but using the 'source' parameter) ...
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compile errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation failed:\n%s\n", infoLog);
        // Don't exit here; return 0 to indicate failure
        return 0;
    }
    return shader;
}

GLuint create_program(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Shader program linking failed:\n%s\n", infoLog);
        exit(EXIT_FAILURE); // Or handle the error more gracefully
    }
    return program;
}

// --- (Your init_quad function, adapted) ---
void init_quad(Renderer *renderer)
{
    // ... (Code from previous example, but use renderer->quadVAO) ...
    const float quadVertices[] = {
        // Positions   // Texture Coords
        -0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 0.0f,

        0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f};
    glGenVertexArrays(1, &renderer->quadVAO);
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(renderer->quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &quadVBO);
}

void init_instance_buffer(Renderer *renderer)
{
    // ... (Code from previous example, but use renderer->instanceSSBO and renderer->maxSprites) ...
    glGenBuffers(1, &renderer->instanceSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderer->instanceSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, renderer->maxSprites * sizeof(Sprite), NULL, GL_DYNAMIC_DRAW); // Allocate enough space
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderer->instanceSSBO);                                // Bind to binding point 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void update_instance_buffer(Renderer *renderer, Sprite *sprites, size_t numSprites)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderer->instanceSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, renderer->maxSprites * sizeof(Sprite), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numSprites * sizeof(Sprite), sprites);

    // Check for errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        fprintf(stderr, "pointer: %p num %d in %d\n", sprites, numSprites, renderer->instanceSSBO);
        fprintf(stderr, "OpenGL error in update_instance_buffer: 0x%04X\n", err);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

int renderer_init(Renderer *renderer, size_t maxSprites, int screenWidth, int screenHeight)
{
    renderer->maxSprites = maxSprites;

    // Initialize quad and instance buffer
    init_quad(renderer);
    init_instance_buffer(renderer);

    // --- Load shaders at runtime ---
    char *vertexShaderSource = read_file_to_string("shaders/sprite.vert");
    char *fragmentShaderSource = read_file_to_string("shaders/sprite.frag");

    if (!vertexShaderSource || !fragmentShaderSource)
    {
        // Error messages are already printed by read_file_to_string
        if (vertexShaderSource)
            free(vertexShaderSource);
        if (fragmentShaderSource)
            free(fragmentShaderSource);
        return 0; // Indicate failure
    }

    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    free(vertexShaderSource); // Free the shader source strings!
    free(fragmentShaderSource);

    if (!vertexShader || !fragmentShader)
    {
        // Don't leak shaders if compilation failed
        if (vertexShader)
            glDeleteShader(vertexShader);
        if (fragmentShader)
            glDeleteShader(fragmentShader);
        return 0; // Indicate failure
    }

    renderer->shaderProgram = create_program(vertexShader, fragmentShader);
    if (!renderer->shaderProgram)
    {
        return 0;
    }

    // Set up projection matrix (once, since it doesn't change)
    mat4x4_ortho(renderer->projection, 0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);

    glUseProgram(renderer->shaderProgram); // Use the program to set uniforms
    glUniformMatrix4fv(glGetUniformLocation(renderer->shaderProgram, "projection"), 1, GL_FALSE, (const GLfloat *)renderer->projection);
    cameraPosLoc = glGetUniformLocation(renderer->shaderProgram, "cameraPos");
    glUseProgram(0); // Unbind

    // Clean up individual shaders (they are linked in the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Creazione della texture array
    GLuint textureArrayID;
    glGenTextures(1, &textureArrayID);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);

    // Supponiamo di avere N texture tutte di dimensione width x height
    int width = 512;
    int height = 512;
    int layers = 4;    // Numero di texture nell'array
    int mipLevels = 1; // Se non vuoi usare mipmapping, altrimenti calcola il valore appropriato
    // Allocazione dello spazio per la texture array
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                   mipLevels,              // Numero di livelli mipmap
                   GL_RGBA8,               // Formato interno
                   width, height, layers); // Dimensioni e numero di layer

    // Caricamento di ogni texture nel suo strato
    for (int i = 0; i < layers; i++)
    {
        char filename[256];
        sprintf(filename, "./assets/textures/%d.png", i);

        int imgWidth, imgHeight, imgChannels;
        unsigned char *imageData = stbi_load(filename, &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);

        if (imageData)
        {
            // Verifica che le dimensioni corrispondano a quelle della texture array
            if (imgWidth == width && imgHeight == height)
            {
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                                0,                // Livello mipmap
                                0, 0, i,          // xOffset, yOffset, zOffset (layer)
                                width, height, 1, // width, height, depth (numero di layer da caricare)
                                GL_RGBA,          // Formato dei dati
                                GL_UNSIGNED_BYTE, // Tipo dei dati
                                imageData);       // Puntatore ai dati
            }
            else
            {
                printf("Texture %d ha dimensioni diverse (%dx%d invece di %dx%d)\n",
                       i, imgWidth, imgHeight, width, height);
                // Qui potresti ridimensionare l'immagine se necessario
            }

            // Libera la memoria dell'immagine dopo averla caricata nella texture
            stbi_image_free(imageData);
        }
        else
        {
            printf("Impossibile caricare la texture %s\n", filename);
        }
    }

    // Impostazione dei parametri di texture senza mipmap
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Generazione delle mipmap (opzionale)
    // glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

    return 1; // Indicate success
}

void renderer_begin_frame(Renderer *renderer)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform2f(cameraPosLoc, game.camera_pos[0], game.camera_pos[1]);
}

void renderer_draw_sprites(Renderer *renderer, Sprite *sprites, size_t numSprites)
{

    update_instance_buffer(renderer, sprites, numSprites);

    glUseProgram(renderer->shaderProgram);
    glBindVertexArray(renderer->quadVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numSprites);
    glBindVertexArray(0);
}

void renderer_end_frame(Renderer *renderer)
{
    // currently empty, but you could add more things to do at the end of a frame
}

void renderer_cleanup(Renderer *renderer)
{
    glDeleteVertexArrays(1, &renderer->quadVAO);
    glDeleteBuffers(1, &renderer->instanceSSBO);
    glDeleteProgram(renderer->shaderProgram);
}

size_t renderer_set_sprites(GameWorld *world, Sprite *drawing)
{
    size_t count = 0;
    memcpy(drawing, world->decorazioni, sizeof(Sprite) * world->count_decorazioni);
    count += world->count_decorazioni;
    // Copia i nemici nel buffer dopo le decorazioni
    //memcpy(drawing + count, enemies, sizeof(Sprite) * world->count_enemies;
    return count;
}