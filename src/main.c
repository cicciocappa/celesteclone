#include <stdlib.h>
#include <stdio.h>

#include "gl_renderer.h"

int main(void)
{
    if (!gl_init())
    {
        fprintf(stderr, "Init ok\n");
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window);
        }

        glfwTerminate();
        return 0;
    }

    return 1;
}
