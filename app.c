#include "oph.h"

#include <stdlib.h>
#include <string.h>

#include "GLFW/glfw3.h"

static bool enable_validation_layers = true;

static void *oph_system_calloc(
    oph_allocator_ctx ctx,
    size_t nmemb,
    size_t size)
{
    void *ptr = calloc(nmemb, size);
    if(!ptr)
    {
        abort();
    }
    return ptr;
}

static void oph_system_free(oph_allocator_ctx ctx, void *ptr)
{
    free(ptr);
}

static struct oph_allocator default_allocator = {
    .ctx = NULL,
    .calloc = oph_system_calloc,
    .free = oph_system_free,
};


static void *oph_calloc(
    struct oph_allocator *allocator,
    size_t nmemb,
    size_t size
    )
{
    return allocator->calloc(allocator->ctx, nmemb, size);
}


static void oph_app_init_vk_physical_device(
    struct oph_application *app,
    VkPhysicalDevice vk_dev,
    struct oph_vk_physical_device *dev
    )
{
    dev->handle = vk_dev;
    vkGetPhysicalDeviceProperties(dev->handle, &dev->properties);
    vkGetPhysicalDeviceFeatures(dev->handle, &dev->features);

    vkGetPhysicalDeviceQueueFamilyProperties(
        dev->handle,
        &dev->n_queue_families,
        NULL);

    dev->queue_families = oph_calloc(
        &app->allocator,
        dev->n_queue_families,
        sizeof(dev->queue_families[0]));

    vkGetPhysicalDeviceQueueFamilyProperties(
        dev->handle,
        &dev->n_queue_families,
        dev->queue_families);

    for(size_t i = 0; i < dev->n_queue_families; ++i)
    {
        if(dev->queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            dev->graphics_queue_family = i;
            dev->have_graphics_queue_family = true;
        }
    }
}

static bool oph_app_is_vk_device_suitable(
    struct oph_application *app,
    struct oph_vk_physical_device *dev
    )
{
    return dev->properties.deviceType ==
           VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           dev->features.geometryShader;
}

static void oph_app_init_vk_physical_devices(struct oph_application *app)
{
    struct oph_vk_backend *vk = &app->backend;

    vkEnumeratePhysicalDevices(
        vk->instance,
        &vk->n_physical_devices,
        NULL);

    app->backend.vk_physical_devices = oph_calloc(
        &app->allocator,
        vk->n_physical_devices,
        sizeof(*app->backend.vk_physical_devices));

    app->backend.physical_devices = oph_calloc(
        &app->allocator,
        vk->n_physical_devices,
        sizeof(*app->backend.physical_devices));

    vkEnumeratePhysicalDevices(
        vk->instance,
        &app->backend.n_physical_devices,
        app->backend.vk_physical_devices);

    for(size_t i = 0; i < app->backend.n_physical_devices; ++i)
    {
        oph_app_init_vk_physical_device(
            app,
            vk->vk_physical_devices[i],
            &vk->physical_devices[i]);

        if(oph_app_is_vk_device_suitable(
            app,
            &vk->physical_devices[i]))
        {
            vk->physical_device = &vk->physical_devices[i];
            break;
        }

        if(app->backend.physical_device)
        {
            abort();
        }
    }
}

void oph_app_init_vk_logical_devices(
    struct oph_application *app,
    struct oph_vk_physical_device *dev,
    struct oph_vk_logical_device *ldev)
{
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = dev->graphics_queue_family;
    queue_create_info.queueCount = 1;

    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.pEnabledFeatures = &dev->features;

    device_create_info.enabledExtensionCount = 0;
    device_create_info.enabledLayerCount = 0;

    VkResult res;

    res = vkCreateDevice(
        dev->handle,
        &device_create_info,
        NULL,
        &ldev->handle);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    vkGetDeviceQueue(
        ldev->handle,
        dev->graphics_queue_family,
        0,
        &ldev->graphics_queue);
}


void oph_app_init(struct oph_application *app)
{
    memset(app, 0, sizeof(*app));

    app->allocator = default_allocator;

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
    if(res)
    {
        abort();
    }

    oph_app_init_vk_physical_devices(app);
    oph_app_init_vk_logical_devices(
        app,
        app->backend.physical_device,
        &app->backend.logical_device);
}

void oph_app_destroy(struct oph_application *app)
{
    vkDestroyInstance(app->backend.instance, NULL);
}