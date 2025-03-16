#include "app.h"

#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "GLFW/glfw3.h"

static bool enable_validation_layers = true;

void oph_app_init(struct oph_application *app)
{
    memset(app, 0, sizeof(*app));

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "oph";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);


    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions = NULL;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    create_info.enabledExtensionCount = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    create_info.enabledLayerCount = 0;

    VkResult res = vkCreateInstance(&create_info, NULL, &app->backend.instance);
    if (res)
    {
        abort();
    }

    oph_device_init(&app->device);
}

void oph_app_destroy(struct oph_application *app)
{
    vkDestroyInstance(app->backend.instance, NULL);
}
