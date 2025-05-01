#include "allocator.h"
#include "oph.h"
#include "sys.h"
#include "vk.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

static bool enable_validation_layers = true;

void oph_app_init_vk_shaders(struct oph_application *app)
{}

void oph_app_init(const struct oph_configuration *config,
                  struct oph_application *app)
{
    memset(app, 0, sizeof(*app));
    oph_sys_init(&config->sys, &app->sys);
    oph_vk_init(&app->sys, &config->vk, &app->vk);
}

void oph_app_destroy(struct oph_application *app)
{
    vkDestroyInstance(app->vk.instance, nullptr);

    oph_sys_destroy(&app->sys);
}