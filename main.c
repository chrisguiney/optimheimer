#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "oph.h"
#include "app.h"
#include "device.h"

#define MAX_MEMORY (128 * 1024 * 1024) // 128mb

void draw(struct nk_context* ctx)
{
    if (
        nk_begin(ctx, "Show", nk_rect(50, 50, 220, 220),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE))
    {
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button"))
        {
            // event handling
        }

    }
}

void tick(GLFWwindow* window, struct nk_context* nk_ctx, struct oph_application *app)
{
    glfwGetWindowSize(window, &app->frame.width, &app->frame.height);
    glfwGetFramebufferSize(window, &app->display.width, &app->display.height);

    glfwPollEvents();

    draw(nk_ctx);

    oph_device_draw(&app->device);

    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    nk_clear(nk_ctx);
    nk_end(nk_ctx);
}

int main(void)
{
    if (!glfwInit())
    {
        abort();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        abort();
    }
    glfwMakeContextCurrent(window);

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

    printf("Extension count: %d\n", extension_count);


    struct oph_application app;
    oph_app_init(&app);

    struct nk_context nk_ctx;
    nk_init_default(&nk_ctx, &app.device.atlas.default_font->handle);



    while (!glfwWindowShouldClose(window))
    {
        tick(window, &nk_ctx, &app);
    }

    glfwTerminate();

    oph_app_destroy(&app);
}
