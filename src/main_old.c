#define GLAD_GL_IMPLEMENTATION
#include "./glad/include/glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include "linmath.h"
 
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
 
typedef struct Vertex
{
    vec2 pos;
    vec3 col;
} Vertex;
 
static const Vertex vertices[3] =
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};
 
static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec3 vCol;\n"
"in vec2 vPos;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";
 
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
 
int main(void)
{
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
 
    // NOTE: OpenGL error checks have been omitted for brevity
 
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
 
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
 
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
 
    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");
 
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, col));
 
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;
 
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
 
        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
 
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

// --- Core Data Structures ---

// Rectangular region in texture atlas
typedef struct {
    float u, v;       // Top-left UV coordinates
    float width;      // Width in UV space
    float height;     // Height in UV space
} AtlasRegion;

// Texture atlas containing multiple sprites
typedef struct {
    GLuint textureId;
    int width, height;
    // Additional metadata if needed
} TextureAtlas;

// Basic sprite definition
typedef struct Sprite Sprite; // Forward declaration for parenting
typedef struct {
    uint32_t id;              // Unique sprite identifier
    AtlasRegion region;       // Region in texture atlas
    float originX, originY;   // Pivot point (0,0 = top-left, 0.5,0.5 = center)
    Sprite* parent;           // Parent sprite (NULL if no parent)
    // Additional sprite properties (animations, etc.)
} SpriteDefinition;

// Instance-specific data for a sprite
typedef struct Sprite {
    SpriteDefinition* definition;
    float x, y;               // Position
    float scaleX, scaleY;     // Scale
    float rotation;           // Rotation in radians
    float r, g, b, a;         // Color/tint and alpha
    int zOrder;               // Render order
    Sprite* parent;           // Parent sprite (NULL if root)
    Sprite** children;        // Array of child sprites
    int childCount;           // Number of children
    int childCapacity;        // Capacity of children array
    mat4 localTransform;      // Cached local transform
    mat4 worldTransform;      // Cached world transform
    bool transformDirty;      // Flag to recalculate transforms
} Sprite;

// --- Sprite Batch Renderer ---

// Data for instanced rendering
typedef struct {
    float posX, posY;         // Final position after transform
    float texU, texV;         // Texture coordinates
    float texW, texH;         // Texture region dimensions
    float r, g, b, a;         // Color data
    // Any other per-instance data
} SpriteInstanceData;

// Batch renderer
typedef struct {
    GLuint vao;               // Vertex Array Object
    GLuint quadVbo;           // VBO for quad vertices
    GLuint instanceVbo;       // VBO for instance data
    GLuint shader;            // Shader program
    TextureAtlas* atlas;      // Current texture atlas
    SpriteInstanceData* instanceData;  // CPU-side buffer for instance data
    int capacity;             // Maximum instances per batch
    int count;                // Current instance count
    mat4 projectionMatrix;    // Projection matrix
} SpriteBatchRenderer;

// --- Function Declarations ---

// Texture Atlas Management
TextureAtlas* createTextureAtlas(const char* filename, int width, int height);
AtlasRegion defineAtlasRegion(TextureAtlas* atlas, int x, int y, int width, int height);
void destroyTextureAtlas(TextureAtlas* atlas);

// Sprite Management
SpriteDefinition* createSpriteDefinition(TextureAtlas* atlas, AtlasRegion region);
Sprite* createSprite(SpriteDefinition* definition);
void setSpriteParent(Sprite* sprite, Sprite* parent);
void addChild(Sprite* parent, Sprite* child);
void removeChild(Sprite* parent, Sprite* child);
void updateSpriteTransform(Sprite* sprite);
void updateSpriteHierarchy(Sprite* root);
void destroySprite(Sprite* sprite);

// Batch Renderer
SpriteBatchRenderer* createSpriteBatchRenderer(int maxSprites, GLuint shader, TextureAtlas* atlas);
void beginBatch(SpriteBatchRenderer* renderer);
void addSpriteToBatch(SpriteBatchRenderer* renderer, Sprite* sprite);
void flushBatch(SpriteBatchRenderer* renderer);
void destroySpriteBatchRenderer(SpriteBatchRenderer* renderer);

// --- Implementation ---

// Creates a sprite batch renderer
SpriteBatchRenderer* createSpriteBatchRenderer(int maxSprites, GLuint shader, TextureAtlas* atlas) {
    SpriteBatchRenderer* renderer = (SpriteBatchRenderer*)malloc(sizeof(SpriteBatchRenderer));
    
    // Setup VAO
    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);
    
    // Setup quad VBO (used for all sprites)
    float quadVertices[] = {
        // positions     // tex coords
        -0.5f, -0.5f,    0.0f, 1.0f,   // bottom left
         0.5f, -0.5f,    1.0f, 1.0f,   // bottom right
         0.5f,  0.5f,    1.0f, 0.0f,   // top right
        -0.5f,  0.5f,    0.0f, 0.0f    // top left
    };
    
    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    glGenBuffers(1, &renderer->quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Setup EBO
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Configure quad vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    // Setup instance VBO
    glGenBuffers(1, &renderer->instanceVbo);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, maxSprites * sizeof(SpriteInstanceData), NULL, GL_DYNAMIC_DRAW);
    
    // Configure instance attributes (location depends on shader design)
    // Position
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstanceData), (void*)offsetof(SpriteInstanceData, posX));
    glVertexAttribDivisor(2, 1);
    
    // Tex coords
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstanceData), (void*)offsetof(SpriteInstanceData, texU));
    glVertexAttribDivisor(3, 1);
    
    // Tex dimensions
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstanceData), (void*)offsetof(SpriteInstanceData, texW));
    glVertexAttribDivisor(4, 1);
    
    // Color
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteInstanceData), (void*)offsetof(SpriteInstanceData, r));
    glVertexAttribDivisor(5, 1);
    
    glBindVertexArray(0);
    
    // Initialize other fields
    renderer->shader = shader;
    renderer->atlas = atlas;
    renderer->capacity = maxSprites;
    renderer->count = 0;
    renderer->instanceData = (SpriteInstanceData*)malloc(maxSprites * sizeof(SpriteInstanceData));
    
    // Default orthographic projection
    renderer->projectionMatrix = ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
    
    return renderer;
}

// Start sprite batch collection
void beginBatch(SpriteBatchRenderer* renderer) {
    renderer->count = 0;
}

// Add sprite to the batch
void addSpriteToBatch(SpriteBatchRenderer* renderer, Sprite* sprite) {
    if (renderer->count >= renderer->capacity) {
        flushBatch(renderer);
    }
    
    // Make sure the transforms are up to date
    if (sprite->transformDirty) {
        updateSpriteTransform(sprite);
    }
    
    // Extract transform data for the instance
    SpriteInstanceData* instance = &renderer->instanceData[renderer->count++];
    
    // Extract position from world transform
    instance->posX = sprite->worldTransform[3][0];
    instance->posY = sprite->worldTransform[3][1];
    
    // Set texture coordinates from atlas region
    instance->texU = sprite->definition->region.u;
    instance->texV = sprite->definition->region.v;
    instance->texW = sprite->definition->region.width;
    instance->texH = sprite->definition->region.height;
    
    // Set color
    instance->r = sprite->r;
    instance->g = sprite->g;
    instance->b = sprite->b;
    instance->a = sprite->a;
}

// Render the batch
void flushBatch(SpriteBatchRenderer* renderer) {
    if (renderer->count == 0) {
        return;
    }
    
    glUseProgram(renderer->shader);
    
    // Bind texture atlas
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->atlas->textureId);
    GLint texUniform = glGetUniformLocation(renderer->shader, "spriteTexture");
    glUniform1i(texUniform, 0);
    
    // Set projection matrix
    GLint projUniform = glGetUniformLocation(renderer->shader, "projection");
    glUniformMatrix4fv(projUniform, 1, GL_FALSE, &renderer->projectionMatrix[0][0]);
    
    // Upload instance data
    glBindBuffer(GL_ARRAY_BUFFER, renderer->instanceVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, renderer->count * sizeof(SpriteInstanceData), renderer->instanceData);
    
    // Render instanced sprites
    glBindVertexArray(renderer->vao);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, renderer->count);
    glBindVertexArray(0);
    
    // Reset counter for next batch
    renderer->count = 0;
}

// Update sprite's transformation matrices
void updateSpriteTransform(Sprite* sprite) {
    // Calculate local transform
    mat4 translation = translate(mat4_identity(), vec3(sprite->x, sprite->y, 0.0f));
    mat4 rotation = rotate(mat4_identity(), sprite->rotation, vec3(0.0f, 0.0f, 1.0f));
    mat4 scale = scale(mat4_identity(), vec3(sprite->scaleX, sprite->scaleY, 1.0f));
    
    // Local transform: scale -> rotate -> translate
    sprite->localTransform = mat4_mul(translation, mat4_mul(rotation, scale));
    
    // World transform depends on parent
    if (sprite->parent) {
        // Ensure parent transform is updated
        if (sprite->parent->transformDirty) {
            updateSpriteTransform(sprite->parent);
        }
        sprite->worldTransform = mat4_mul(sprite->parent->worldTransform, sprite->localTransform);
    } else {
        sprite->worldTransform = sprite->localTransform;
    }
    
    sprite->transformDirty = false;
    
    // Mark all children as dirty
    for (int i = 0; i < sprite->childCount; i++) {
        sprite->children[i]->transformDirty = true;
    }
}

// Recursively update all sprites in a hierarchy
void updateSpriteHierarchy(Sprite* root) {
    if (root->transformDirty) {
        updateSpriteTransform(root);
    }
    
    for (int i = 0; i < root->childCount; i++) {
        updateSpriteHierarchy(root->children[i]);
    }
}

// Parent-child relationship management
void setSpriteParent(Sprite* sprite, Sprite* parent) {
    // Remove from previous parent if any
    if (sprite->parent) {
        removeChild(sprite->parent, sprite);
    }
    
    sprite->parent = parent;
    
    // Add to new parent if provided
    if (parent) {
        addChild(parent, sprite);
    }
    
    // Mark transform as dirty to update world transform
    sprite->transformDirty = true;
}

// Add child to parent sprite
void addChild(Sprite* parent, Sprite* child) {
    // Ensure capacity
    if (parent->childCount >= parent->childCapacity) {
        parent->childCapacity = parent->childCapacity == 0 ? 4 : parent->childCapacity * 2;
        parent->children = (Sprite**)realloc(parent->children, 
                                             parent->childCapacity * sizeof(Sprite*));
    }
    
    // Add child
    parent->children[parent->childCount++] = child;
    child->parent = parent;
    
    // Mark child's transform as dirty
    child->transformDirty = true;
}

// Remove child from parent
void removeChild(Sprite* parent, Sprite* child) {
    for (int i = 0; i < parent->childCount; i++) {
        if (parent->children[i] == child) {
            // Shift all children after this one
            for (int j = i; j < parent->childCount - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->childCount--;
            child->parent = NULL;
            
            // Mark child's transform as dirty
            child->transformDirty = true;
            return;
        }
    }
}

// Create a new sprite from a definition
Sprite* createSprite(SpriteDefinition* definition) {
    Sprite* sprite = (Sprite*)malloc(sizeof(Sprite));
    
    sprite->definition = definition;
    sprite->x = sprite->y = 0.0f;
    sprite->scaleX = sprite->scaleY = 1.0f;
    sprite->rotation = 0.0f;
    sprite->r = sprite->g = sprite->b = sprite->a = 1.0f;
    sprite->zOrder = 0;
    sprite->parent = NULL;
    sprite->children = NULL;
    sprite->childCount = 0;
    sprite->childCapacity = 0;
    sprite->localTransform = mat4_identity();
    sprite->worldTransform = mat4_identity();
    sprite->transformDirty = true;
    
    return sprite;
}

// Cleanup sprite resources
void destroySprite(Sprite* sprite) {
    // Remove all children
    for (int i = 0; i < sprite->childCount; i++) {
        sprite->children[i]->parent = NULL;
    }
    
    // Remove from parent if any
    if (sprite->parent) {
        removeChild(sprite->parent, sprite);
    }
    
    // Free children array
    if (sprite->children) {
        free(sprite->children);
    }
    
    // Free sprite struct
    free(sprite);
}

// Vertex Shader for Sprites
#version 330 core
layout (location = 0) in vec2 aPos;         // Quad vertex position
layout (location = 1) in vec2 aTexCoord;    // Quad texture coordinates
layout (location = 2) in vec2 aInstPos;     // Instance position
layout (location = 3) in vec2 aTexUV;       // Instance texture coordinates (top-left)
layout (location = 4) in vec2 aTexWH;       // Instance texture dimensions
layout (location = 5) in vec4 aColor;       // Instance color

uniform mat4 projection;

out vec2 TexCoord;
out vec4 Color;

void main() {
    // Calculate final position
    vec2 pos = aPos;
    
    // Scale vertex by texture region size
    pos.x *= aTexWH.x;
    pos.y *= aTexWH.y;
    
    // Apply instance position
    pos += aInstPos;
    
    gl_Position = projection * vec4(pos, 0.0, 1.0);
    
    // Calculate texture coordinates based on region
    TexCoord = aTexUV + aTexCoord * aTexWH;
    
    // Pass color to fragment shader
    Color = aColor;
}

// Fragment Shader for Sprites
#version 330 core
in vec2 TexCoord;
in vec4 Color;

uniform sampler2D spriteTexture;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(spriteTexture, TexCoord);
    FragColor = texColor * Color;
}

// Example usage of the sprite system

// Initialize OpenGL and create window (platform-specific code omitted)

// Load shader program
GLuint shaderProgram = createShaderProgram("sprite.vs", "sprite.fs");

// Create texture atlas
TextureAtlas* atlas = createTextureAtlas("spritesheet.png", 1024, 1024);

// Define sprite regions
AtlasRegion playerRegion = defineAtlasRegion(atlas, 0, 0, 64, 64);
AtlasRegion weaponRegion = defineAtlasRegion(atlas, 64, 0, 32, 32);

// Create sprite definitions
SpriteDefinition* playerDef = createSpriteDefinition(atlas, playerRegion);
SpriteDefinition* weaponDef = createSpriteDefinition(atlas, weaponRegion);

// Create sprite instances
Sprite* playerSprite = createSprite(playerDef);
playerSprite->x = 400;
playerSprite->y = 300;

Sprite* weaponSprite = createSprite(weaponDef);
weaponSprite->x = 20; // Relative to parent (player)
weaponSprite->y = 0;  // Relative to parent (player)

// Set up parent-child relationship
setSpriteParent(weaponSprite, playerSprite);

// Create batch renderer
SpriteBatchRenderer* batchRenderer = createSpriteBatchRenderer(1000, shaderProgram, atlas);

// Main game loop
while (!windowShouldClose()) {
    // Update game logic, move sprites, etc.
    updateSpriteHierarchy(playerSprite);  // Updates all transformations in the hierarchy
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render sprites
    beginBatch(batchRenderer);
    
    // Sprite rendering is depth-sorted for proper overlapping
    addSpriteToBatch(batchRenderer, playerSprite);
    addSpriteToBatch(batchRenderer, weaponSprite);
    
    // Add more sprites here...
    
    flushBatch(batchRenderer);
    
    // Swap buffers, poll events
    swapBuffers();
    pollEvents();
}

// Cleanup
destroySprite(weaponSprite);
destroySprite(playerSprite);
destroySpriteBatchRenderer(batchRenderer);
destroyTextureAtlas(atlas);

// --- Spatial Partitioning: Grid System ---

// Grid cell containing sprites
typedef struct GridCell {
    Sprite** sprites;        // Dynamic array of sprites in this cell
    int count;               // Number of sprites in this cell
    int capacity;            // Capacity of sprite array
} GridCell;

// Grid system for spatial partitioning
typedef struct SpatialGrid {
    GridCell** cells;        // 2D array of grid cells
    int width;               // Width of grid (in cells)
    int height;              // Height of grid (in cells)
    float cellWidth;         // Width of each cell (in world units)
    float cellHeight;        // Height of each cell (in world units)
    float worldX;            // World X coordinate of grid origin
    float worldY;            // World Y coordinate of grid origin
    float worldWidth;        // Total width of grid in world units
    float worldHeight;       // Total height of grid in world units
} SpatialGrid;

// --- Function Declarations ---

SpatialGrid* createSpatialGrid(float worldX, float worldY, float worldWidth, float worldHeight, int cellsX, int cellsY);
void destroySpatialGrid(SpatialGrid* grid);
void addSpriteToGrid(SpatialGrid* grid, Sprite* sprite);
void removeSpriteFromGrid(SpatialGrid* grid, Sprite* sprite);
void updateSpriteInGrid(SpatialGrid* grid, Sprite* sprite);
void getSpriteGridPosition(SpatialGrid* grid, Sprite* sprite, int* cellX, int* cellY);
GridCell* getGridCell(SpatialGrid* grid, int cellX, int cellY);
void getVisibleCells(SpatialGrid* grid, float viewX, float viewY, float viewWidth, float viewHeight, int* minCellX, int* minCellY, int* maxCellX, int* maxCellY);
void renderVisibleSprites(SpatialGrid* grid, SpriteBatchRenderer* renderer, float viewX, float viewY, float viewWidth, float viewHeight);

// --- Implementation ---

// Create a new spatial grid
SpatialGrid* createSpatialGrid(float worldX, float worldY, float worldWidth, float worldHeight, int cellsX, int cellsY) {
    SpatialGrid* grid = (SpatialGrid*)malloc(sizeof(SpatialGrid));
    
    grid->worldX = worldX;
    grid->worldY = worldY;
    grid->worldWidth = worldWidth;
    grid->worldHeight = worldHeight;
    grid->width = cellsX;
    grid->height = cellsY;
    grid->cellWidth = worldWidth / cellsX;
    grid->cellHeight = worldHeight / cellsY;
    
    // Allocate the 2D array of cells
    grid->cells = (GridCell**)malloc(cellsY * sizeof(GridCell*));
    for (int y = 0; y < cellsY; y++) {
        grid->cells[y] = (GridCell*)malloc(cellsX * sizeof(GridCell));
        for (int x = 0; x < cellsX; x++) {
            grid->cells[y][x].sprites = NULL;
            grid->cells[y][x].count = 0;
            grid->cells[y][x].capacity = 0;
        }
    }
    
    return grid;
}

// Free resources for a spatial grid
void destroySpatialGrid(SpatialGrid* grid) {
    for (int y = 0; y < grid->height; y++) {
        for (int x = 0; x < grid->width; x++) {
            if (grid->cells[y][x].sprites) {
                free(grid->cells[y][x].sprites);
            }
        }
        free(grid->cells[y]);
    }
    free(grid->cells);
    free(grid);
}

// Calculate which grid cell a sprite belongs to based on its position
void getSpriteGridPosition(SpatialGrid* grid, Sprite* sprite, int* cellX, int* cellY) {
    // Ensure transforms are up to date
    if (sprite->transformDirty) {
        updateSpriteTransform(sprite);
    }
    
    // Get world position from transform matrix
    float worldX = sprite->worldTransform[3][0];
    float worldY = sprite->worldTransform[3][1];
    
    // Calculate grid cell position
    *cellX = (int)((worldX - grid->worldX) / grid->cellWidth);
    *cellY = (int)((worldY - grid->worldY) / grid->cellHeight);
    
    // Clamp to grid boundaries
    if (*cellX < 0) *cellX = 0;
    if (*cellX >= grid->width) *cellX = grid->width - 1;
    if (*cellY < 0) *cellY = 0;
    if (*cellY >= grid->height) *cellY = grid->height - 1;
}

// Get a grid cell at the specified coordinates
GridCell* getGridCell(SpatialGrid* grid, int cellX, int cellY) {
    // Check bounds
    if (cellX < 0 || cellX >= grid->width || cellY < 0 || cellY >= grid->height) {
        return NULL;
    }
    return &grid->cells[cellY][cellX];
}

// Add a sprite to the appropriate grid cell
void addSpriteToGrid(SpatialGrid* grid, Sprite* sprite) {
    int cellX, cellY;
    getSpriteGridPosition(grid, sprite, &cellX, &cellY);
    
    GridCell* cell = getGridCell(grid, cellX, cellY);
    if (!cell) return;
    
    // Expand capacity if needed
    if (cell->count >= cell->capacity) {
        cell->capacity = cell->capacity == 0 ? 4 : cell->capacity * 2;
        cell->sprites = (Sprite**)realloc(cell->sprites, cell->capacity * sizeof(Sprite*));
    }
    
    // Add sprite to cell
    cell->sprites[cell->count++] = sprite;
    
    // Store grid cell information in sprite for quick removal/updates
    // This requires adding cellX and cellY fields to the Sprite struct
    sprite->cellX = cellX;
    sprite->cellY = cellY;
}

// Remove a sprite from the grid
void removeSpriteFromGrid(SpatialGrid* grid, Sprite* sprite) {
    GridCell* cell = getGridCell(grid, sprite->cellX, sprite->cellY);
    if (!cell) return;
    
    // Find and remove sprite from cell
    for (int i = 0; i < cell->count; i++) {
        if (cell->sprites[i] == sprite) {
            // Shift remaining sprites down
            for (int j = i; j < cell->count - 1; j++) {
                cell->sprites[j] = cell->sprites[j + 1];
            }
            cell->count--;
            return;
        }
    }
}

// Update a sprite's position in the grid (remove and re-add if needed)
void updateSpriteInGrid(SpatialGrid* grid, Sprite* sprite) {
    int newCellX, newCellY;
    getSpriteGridPosition(grid, sprite, &newCellX, &newCellY);
    
    // If the sprite has moved to a different cell, update its position
    if (newCellX != sprite->cellX || newCellY != sprite->cellY) {
        removeSpriteFromGrid(grid, sprite);
        sprite->cellX = newCellX;
        sprite->cellY = newCellY;
        addSpriteToGrid(grid, sprite);
    }
}

// Calculate the range of grid cells visible in the current view
void getVisibleCells(SpatialGrid* grid, float viewX, float viewY, float viewWidth, float viewHeight,
                    int* minCellX, int* minCellY, int* maxCellX, int* maxCellY) {
    // Convert view bounds to grid cell coordinates
    *minCellX = (int)((viewX - grid->worldX) / grid->cellWidth);
    *minCellY = (int)((viewY - grid->worldY) / grid->cellHeight);
    *maxCellX = (int)((viewX + viewWidth - grid->worldX) / grid->cellWidth);
    *maxCellY = (int)((viewY + viewHeight - grid->worldY) / grid->cellHeight);
    
    // Clamp to grid boundaries
    if (*minCellX < 0) *minCellX = 0;
    if (*minCellY < 0) *minCellY = 0;
    if (*maxCellX >= grid->width) *maxCellX = grid->width - 1;
    if (*maxCellY >= grid->height) *maxCellY = grid->height - 1;
}

// Render only the sprites visible in the current view
void renderVisibleSprites(SpatialGrid* grid, SpriteBatchRenderer* renderer, 
                         float viewX, float viewY, float viewWidth, float viewHeight) {
    int minCellX, minCellY, maxCellX, maxCellY;
    getVisibleCells(grid, viewX, viewY, viewWidth, viewHeight,
                   &minCellX, &minCellY, &maxCellX, &maxCellY);
    
    beginBatch(renderer);
    
    // Collect all visible sprites and add them to the batch
    for (int y = minCellY; y <= maxCellY; y++) {
        for (int x = minCellX; x <= maxCellX; x++) {
            GridCell* cell = getGridCell(grid, x, y);
            if (!cell) continue;
            
            for (int i = 0; i < cell->count; i++) {
                Sprite* sprite = cell->sprites[i];
                addSpriteToBatch(renderer, sprite);
            }
        }
    }
    
    flushBatch(renderer);
}

// --- Modifications to Sprite struct ---

typedef struct Sprite {
    // Existing fields...
    
    // Grid cell position (for spatial partitioning)
    int cellX, cellY;
    
    // Rest of existing fields...
} Sprite;

// --- Example usage with spatial grid ---

// Initialize our spatial grid
SpatialGrid* grid = createSpatialGrid(0, 0, 10000, 8000, 50, 40);  // 50x40 grid over 10000x8000 world

// When creating or moving sprites, add/update them in the grid
Sprite* newSprite = createSprite(someSpriteDefinition);
newSprite->x = 1500;
newSprite->y = 1200;
updateSpriteTransform(newSprite);
addSpriteToGrid(grid, newSprite);

// In the main game loop
void gameLoop() {
    // Camera/view info
    float cameraX = 500;
    float cameraY = 400;
    float viewWidth = 800;
    float viewHeight = 600;
    
    // Update sprites
    for (int i = 0; i < activeSpritesCount; i++) {
        Sprite* sprite = activeSprites[i];
        
        // Update sprite logic, position, etc.
        
        // Update transforms if needed
        if (sprite->transformDirty) {
            updateSpriteTransform(sprite);
        }
        
        // Update sprite in the grid if it moved
        updateSpriteInGrid(grid, sprite);
    }
    
    // Render only visible sprites
    renderVisibleSprites(grid, batchRenderer, cameraX, cameraY, viewWidth, viewHeight);
}

// Clean up a sprite (remember to remove from grid)
void cleanupSprite(Sprite* sprite) {
    removeSpriteFromGrid(grid, sprite);
    destroySprite(sprite);
}


// --- Animation System ---

// Single frame in an animation
typedef struct {
    AtlasRegion region;      // Texture region for this frame
    float duration;          // Duration of frame in seconds
} AnimationFrame;

// Complete animation sequence
typedef struct {
    char* name;              // Animation name (e.g., "walk", "jump", "idle")
    AnimationFrame* frames;  // Array of animation frames
    int frameCount;          // Number of frames in animation
    bool looping;            // Whether the animation should loop
} AnimationSequence;

// Container for all animations for a sprite
typedef struct {
    AnimationSequence* sequences;  // Array of animation sequences
    int sequenceCount;             // Number of sequences
    int currentSequence;           // Index of current animation sequence
    int currentFrame;              // Index of current frame in sequence
    float timeAccumulated;         // Time accumulated on current frame
    bool playing;                  // Whether animation is currently playing
} AnimationController;

// Add these fields to the Sprite struct
typedef struct Sprite {
    // Existing fields...
    AnimationController* animations;  // Animation controller (NULL if no animations)
    // Rest of existing fields...
} Sprite;

// --- Function Declarations ---

// Animation creation and management
AnimationController* createAnimationController();
void destroyAnimationController(AnimationController* controller);
AnimationSequence* createAnimationSequence(const char* name, int frameCapacity, bool looping);
void addFrameToSequence(AnimationSequence* sequence, AtlasRegion region, float duration);
void addAnimationSequence(AnimationController* controller, AnimationSequence* sequence);

// Animation playback control
void playAnimation(Sprite* sprite, const char* name, bool restart);
void stopAnimation(Sprite* sprite);
void pauseAnimation(Sprite* sprite);
void resumeAnimation(Sprite* sprite);
void updateAnimation(Sprite* sprite, float deltaTime);

// --- Implementation ---

// Create a new animation controller
AnimationController* createAnimationController() {
    AnimationController* controller = (AnimationController*)malloc(sizeof(AnimationController));
    controller->sequences = NULL;
    controller->sequenceCount = 0;
    controller->currentSequence = -1;  // No active sequence
    controller->currentFrame = 0;
    controller->timeAccumulated = 0.0f;
    controller->playing = false;
    return controller;
}

// Create a new animation sequence
AnimationSequence* createAnimationSequence(const char* name, int frameCapacity, bool looping) {
    AnimationSequence* sequence = (AnimationSequence*)malloc(sizeof(AnimationSequence));
    sequence->name = strdup(name);
    sequence->frames = (AnimationFrame*)malloc(frameCapacity * sizeof(AnimationFrame));
    sequence->frameCount = 0;
    sequence->looping = looping;
    return sequence;
}

// Add a frame to an animation sequence
void addFrameToSequence(AnimationSequence* sequence, AtlasRegion region, float duration) {
    sequence->frames[sequence->frameCount].region = region;
    sequence->frames[sequence->frameCount].duration = duration;
    sequence->frameCount++;
}

// Add an animation sequence to a controller
void addAnimationSequence(AnimationController* controller, AnimationSequence* sequence) {
    // Expand capacity if needed
    controller->sequences = (AnimationSequence*)realloc(
        controller->sequences, 
        (controller->sequenceCount + 1) * sizeof(AnimationSequence)
    );
    
    controller->sequences[controller->sequenceCount] = *sequence;
    controller->sequenceCount++;
    
    // Free the original pointer since we've copied the data
    free(sequence);
}

// Find animation sequence by name
int findAnimationSequence(AnimationController* controller, const char* name) {
    for (int i = 0; i < controller->sequenceCount; i++) {
        if (strcmp(controller->sequences[i].name, name) == 0) {
            return i;
        }
    }
    return -1;  // Not found
}

// Play an animation by name
void playAnimation(Sprite* sprite, const char* name, bool restart) {
    if (!sprite->animations) return;
    
    AnimationController* controller = sprite->animations;
    int sequenceIndex = findAnimationSequence(controller, name);
    
    if (sequenceIndex < 0) return;  // Sequence not found
    
    // If already playing this sequence and restart is false, do nothing
    if (controller->currentSequence == sequenceIndex && controller->playing && !restart) {
        return;
    }
    
    controller->currentSequence = sequenceIndex;
    controller->currentFrame = 0;
    controller->timeAccumulated = 0.0f;
    controller->playing = true;
    
    // Update sprite's texture region to first frame
    AnimationSequence* sequence = &controller->sequences[controller->currentSequence];
    sprite->definition->region = sequence->frames[0].region;
}

// Stop an animation
void stopAnimation(Sprite* sprite) {
    if (!sprite->animations) return;
    
    AnimationController* controller = sprite->animations;
    controller->playing = false;
    controller->currentFrame = 0;
    controller->timeAccumulated = 0.0f;
    
    // Reset to first frame of current sequence if one is active
    if (controller->currentSequence >= 0) {
        AnimationSequence* sequence = &controller->sequences[controller->currentSequence];
        sprite->definition->region = sequence->frames[0].region;
    }
}

// Pause animation playback
void pauseAnimation(Sprite* sprite) {
    if (!sprite->animations) return;
    sprite->animations->playing = false;
}

// Resume animation playback
void resumeAnimation(Sprite* sprite) {
    if (!sprite->animations) return;
    sprite->animations->playing = true;
}

// Update animation based on elapsed time
void updateAnimation(Sprite* sprite, float deltaTime) {
    if (!sprite->animations || !sprite->animations->playing) return;
    
    AnimationController* controller = sprite->animations;
    
    // Get current sequence
    if (controller->currentSequence < 0 || 
        controller->currentSequence >= controller->sequenceCount) {
        return;
    }
    
    AnimationSequence* sequence = &controller->sequences[controller->currentSequence];
    
    // Accumulate time
    controller->timeAccumulated += deltaTime;
    
    // Check if we need to advance frames
    AnimationFrame* currentFrame = &sequence->frames[controller->currentFrame];
    while (controller->timeAccumulated >= currentFrame->duration) {
        // Consume the time for this frame
        controller->timeAccumulated -= currentFrame->duration;
        
        // Move to next frame
        controller->currentFrame++;
        
        // Handle end of sequence
        if (controller->currentFrame >= sequence->frameCount) {
            if (sequence->looping) {
                // Loop back to start
                controller->currentFrame = 0;
            } else {
                // Stop at last frame
                controller->currentFrame = sequence->frameCount - 1;
                controller->playing = false;
                break;
            }
        }
        
        // Update current frame pointer
        currentFrame = &sequence->frames[controller->currentFrame];
    }
    
    // Update sprite's texture region to current frame
    sprite->definition->region = sequence->frames[controller->currentFrame].region;
}

// Clean up animation controller resources
void destroyAnimationController(AnimationController* controller) {
    if (!controller) return;
    
    // Free all sequences
    for (int i = 0; i < controller->sequenceCount; i++) {
        free(controller->sequences[i].name);
        free(controller->sequences[i].frames);
    }
    
    free(controller->sequences);
    free(controller);
}

// --- Modifications to Sprite struct and related functions ---

// Add the animations field to the Sprite struct (already shown in the animation system)

// Modify the createSprite function to initialize animation controller
Sprite* createSprite(SpriteDefinition* definition) {
    Sprite* sprite = (Sprite*)malloc(sizeof(Sprite));
    
    // Initialize basic properties
    sprite->definition = definition;
    sprite->x = sprite->y = 0.0f;
    sprite->scaleX = sprite->scaleY = 1.0f;
    sprite->rotation = 0.0f;
    sprite->r = sprite->g = sprite->b = sprite->a = 1.0f;
    sprite->zOrder = 0;
    sprite->parent = NULL;
    sprite->children = NULL;
    sprite->childCount = 0;
    sprite->childCapacity = 0;
    sprite->localTransform = mat4_identity();
    sprite->worldTransform = mat4_identity();
    sprite->transformDirty = true;
    
    // Initialize animation controller to NULL
    sprite->animations = NULL;
    
    // Initialize grid cell position
    sprite->cellX = sprite->cellY = -1;
    
    return sprite;
}

// Modify destroySprite to clean up animation resources
void destroySprite(Sprite* sprite) {
    // Remove all children
    for (int i = 0; i < sprite->childCount; i++) {
        sprite->children[i]->parent = NULL;
    }
    
    // Remove from parent if any
    if (sprite->parent) {
        removeChild(sprite->parent, sprite);
    }
    
    // Destroy animation controller if it exists
    if (sprite->animations) {
        destroyAnimationController(sprite->animations);
    }
    
    // Free children array
    if (sprite->children) {
        free(sprite->children);
    }
    
    // Free sprite struct
    free(sprite);
}

// --- Example Usage ---

// Setup player animation sequences
void setupPlayerAnimations(TextureAtlas* atlas, Sprite* playerSprite) {
    // Create animation controller
    playerSprite->animations = createAnimationController();
    
    // Create "idle" animation sequence (4 frames)
    AnimationSequence* idleAnim = createAnimationSequence("idle", 4, true);
    addFrameToSequence(idleAnim, defineAtlasRegion(atlas, 0, 0, 64, 64), 0.25f);
    addFrameToSequence(idleAnim, defineAtlasRegion(atlas, 64, 0, 64, 64), 0.25f);
    addFrameToSequence(idleAnim, defineAtlasRegion(atlas, 128, 0, 64, 64), 0.25f);
    addFrameToSequence(idleAnim, defineAtlasRegion(atlas, 192, 0, 64, 64), 0.25f);
    addAnimationSequence(playerSprite->animations, idleAnim);
    
    // Create "run" animation sequence (8 frames)
    AnimationSequence* runAnim = createAnimationSequence("run", 8, true);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 0, 64, 64, 64), 0.1f);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 64, 64, 64, 64), 0.1f);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 128, 64, 64, 64), 0.1f);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 192, 64, 64, 64), 0.1f);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 256, 64, 64, 64), 0.1f);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 320, 64, 64, 64), 0.1f);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 384, 64, 64, 64), 0.1f);
    addFrameToSequence(runAnim, defineAtlasRegion(atlas, 448, 64, 64, 64), 0.1f);
    addAnimationSequence(playerSprite->animations, runAnim);
    
    // Create "jump" animation sequence (one-shot, non-looping)
    AnimationSequence* jumpAnim = createAnimationSequence("jump", 4, false);
    addFrameToSequence(jumpAnim, defineAtlasRegion(atlas, 0, 128, 64, 64), 0.1f);
    addFrameToSequence(jumpAnim, defineAtlasRegion(atlas, 64, 128, 64, 64), 0.2f);
    addFrameToSequence(jumpAnim, defineAtlasRegion(atlas, 128, 128, 64, 64), 0.3f);
    addFrameToSequence(jumpAnim, defineAtlasRegion(atlas, 192, 128, 64, 64), 0.2f);
    addAnimationSequence(playerSprite->animations, jumpAnim);
    
    // Start with idle animation by default
    playAnimation(playerSprite, "idle", true);
}

// Main game loop with animation updates
void gameLoop() {
    float currentTime = getTime();  // Platform-specific time function
    static float lastTime = 0;
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Get input
    bool isRunning = isKeyPressed(KEY_RIGHT) || isKeyPressed(KEY_LEFT);
    bool isJumping = isKeyPressed(KEY_SPACE) && !playerIsInAir;
    
    // Update player animation based on state
    if (isJumping) {
        playAnimation(playerSprite, "jump", true);  // Always restart jump animation
    } else if (isRunning) {
        playAnimation(playerSprite, "run", false);  // Don't restart if already running
    } else {
        playAnimation(playerSprite, "idle", false); // Return to idle when not moving
    }
    
    // Update all sprite animations
    for (int i = 0; i < activeSpritesCount; i++) {
        updateAnimation(activeSprites[i], deltaTime);
    }
    
    // Update transforms, grid positions, etc.
    updateSpatialGrid(); // As implemented in the previous section
    
    // Render visible sprites
    renderVisibleSprites(); // As implemented in the previous section
}