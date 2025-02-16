#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "../glad/glad.h" // This should include glad.h internally
#include <GLFW/glfw3.h>



const GLuint WIDTH = 800, HEIGHT = 600;

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                              GLsizei length, const GLchar *message, const void *userParam) {
    (void)length; (void)userParam; (void)id;
    
    fprintf(stderr, "OpenGL Debug Message:\n");
    fprintf(stderr, "Source: 0x%x\n", source);
    fprintf(stderr, "Type: 0x%x\n", type);
    fprintf(stderr, "Severity: 0x%x\n", severity);
    fprintf(stderr, "Message: %s\n", message);

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        fprintf(stderr, "Critical error detected, terminating...\n");
        exit(EXIT_FAILURE);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

typedef struct
{
    GLuint programID;
} GlContext;

static GlContext glContext;
static GLFWwindow *window;

 

bool gl_init()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "[glad] GL with GLFW", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return false;
    }

    //printf("GL %d.%d\n", GLAD_VERSION_MAJOR(GLAD_GL_VERSION), GLAD_VERSION_MINOR(GLAD_GL_VERSION));

    // Enable OpenGL debugging
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }

    
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
}