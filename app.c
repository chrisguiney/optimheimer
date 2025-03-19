#include "oph.h"

#include <stdlib.h>
#include <string.h>

#include "GLFW/glfw3.h"

#include <assert.h>

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
    .ctx = nullptr,
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


static bool oph_app_is_vk_device_suitable(
    struct oph_application *app,
    struct oph_vk_physical_device *dev
    )
{
    return
        dev->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        dev->features.geometryShader &&
        dev->n_surface_formats > 0 && dev->n_present_modes > 0;
}

static VkSurfaceFormatKHR oph_app_select_vk_surface_format(
    struct oph_application *app,
    struct oph_vk_physical_device *dev
    )
{
    assert(dev->n_surface_formats > 0);
    for(size_t i = 0; i < dev->n_surface_formats; ++i)
    {
        VkSurfaceFormatKHR f = dev->surface_formats[i];
        if(f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace ==
           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return f;
        }
    }
    return dev->surface_formats[0];
}

static VkPresentModeKHR oph_app_select_vk_swap_present_mode()
{
    return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t clamp_uint32(uint32_t x, uint32_t min, uint32_t max)
{
    if(x < min)
    {
        return min;
    }
    if(x > max)
    {
        return max;
    }
    return x;
}

static VkExtent2D oph_app_get_vk_extent(
    struct oph_application *app,
    const VkSurfaceCapabilitiesKHR *capabilities
    )
{
    if(capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }

    VkExtent2D extent = {
        .width = clamp_uint32(
            app->presentation.framebuffer_width,
            capabilities->minImageExtent.width,
            capabilities->maxImageExtent.width),

        .height = clamp_uint32(
            app->presentation.framebuffer_height,
            capabilities->minImageExtent.height,
            capabilities->maxImageExtent.height)
    };

    return extent;
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

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        dev->handle,
        dev->surface,
        &dev->surface_capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        dev->handle,
        dev->surface,
        &dev->n_surface_formats,
        nullptr);

    dev->surface_formats = oph_calloc(
        &app->allocator,
        dev->n_surface_formats,
        sizeof(VkSurfaceFormatKHR));

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        dev->handle,
        dev->surface,
        &dev->n_surface_formats,
        dev->surface_formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        dev->handle,
        dev->surface,
        &dev->n_present_modes,
        nullptr);

    dev->present_modes = oph_calloc(
        &app->allocator,
        dev->n_present_modes,
        sizeof(*dev->present_modes));

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        dev->handle,
        dev->surface,
        &dev->n_present_modes,
        dev->present_modes);

    VkResult res = glfwCreateWindowSurface(
        app->backend.instance,
        app->presentation.window,
        nullptr,
        &dev->surface);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    vkEnumerateDeviceExtensionProperties(
        dev->handle,
        nullptr,
        &dev->n_extension_properties,
        nullptr);

    dev->extension_properties = oph_calloc(
        &app->allocator,
        dev->n_extension_properties,
        sizeof(*dev->extension_properties));

    vkEnumerateDeviceExtensionProperties(
        dev->handle,
        nullptr,
        &dev->n_extension_properties,
        dev->extension_properties);

    vkGetPhysicalDeviceQueueFamilyProperties(
        dev->handle,
        &dev->n_queue_families,
        nullptr);

    dev->queue_families = oph_calloc(
        &app->allocator,
        dev->n_queue_families,
        sizeof(*dev->queue_families));

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

        vkGetPhysicalDeviceSurfaceSupportKHR(
            dev->handle,
            i,
            dev->surface,
            &dev->have_present_queue_family);
    }
}

static void oph_app_init_vk_physical_devices(struct oph_application *app)
{
    struct oph_vk_backend *vk = &app->backend;

    vkEnumeratePhysicalDevices(
        vk->instance,
        &vk->n_physical_devices,
        nullptr);

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

static const char *device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static void oph_app_init_vk_logical_devices(
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

    device_create_info.enabledExtensionCount =
        sizeof(device_extensions) / sizeof(device_extensions[0]);
    device_create_info.ppEnabledExtensionNames = device_extensions;

    device_create_info.enabledLayerCount = 0;

    VkResult res = VK_SUCCESS;

    res = vkCreateDevice(
        dev->handle,
        &device_create_info,
        nullptr,
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

static void oph_app_init_presentation(
    const struct oph_configuration *config,
    struct oph_application *app
    )
{
    app->presentation.window = glfwCreateWindow(
        config->initial_window_width,
        config->initial_window_height,
        "Optimheimer",
        nullptr,
        nullptr);
    if(!app->presentation.window)
    {
        abort();
    }

    glfwGetFramebufferSize(
        app->presentation.window,
        &app->presentation.framebuffer_width,
        &app->presentation.framebuffer_height);
}

static void oph_app_init_vk_imageview(
    struct oph_vk_logical_device *ldev,
    struct oph_vk_swapchain *swapchain,
    uint32_t i
    )
{
    VkImageViewCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = swapchain->images[i];
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = swapchain->surface_format.format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    VkResult res = vkCreateImageView(
        ldev->handle,
        &create_info,
        nullptr,
        &swapchain->image_views[i]);

    if(res != VK_SUCCESS)
    {
        abort();
    }
}

static void oph_app_init_vk_swapchain(struct oph_application *app)
{
    app->swapchain.extent = oph_app_get_vk_extent(
        app,
        &app->backend.physical_device->surface_capabilities);

    app->swapchain.surface_format = oph_app_select_vk_surface_format(
        app,
        app->backend.physical_device);

    app->swapchain.present_mode = oph_app_select_vk_swap_present_mode();

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = app->backend.physical_device->surface;
    create_info.minImageCount = app->backend.physical_device->
                                     surface_capabilities.minImageCount;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t queue_family_indices[] = {
        app->backend.physical_device->graphics_queue_family,
        app->backend.physical_device->present_queue_family
    };

    if(queue_family_indices[0] != queue_family_indices[1])
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = app->backend.physical_device->
                                    surface_capabilities.currentTransform;

    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = app->swapchain.present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(
        app->backend.logical_device.handle,
        &create_info,
        nullptr,
        &app->swapchain.handle);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    res = vkGetSwapchainImagesKHR(
        app->backend.logical_device.handle,
        app->swapchain.handle,
        &app->swapchain.n_images,
        nullptr);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    app->swapchain.images = oph_calloc(
        &app->allocator,
        app->swapchain.n_images,
        sizeof(*app->swapchain.images));

    res = vkGetSwapchainImagesKHR(
        app->backend.logical_device.handle,
        app->swapchain.handle,
        &app->swapchain.n_images,
        app->swapchain.images);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    app->swapchain.image_views = oph_calloc(
        &app->allocator,
        app->swapchain.n_images,
        sizeof(*app->swapchain.image_views));

    for(size_t i = 0; i < app->swapchain.n_images; ++i)
    {
        oph_app_init_vk_imageview(
            &app->backend.logical_device,
            &app->swapchain,
            i);
    }
}


void oph_app_init(
    const struct oph_configuration *config,
    struct oph_application *app
    )
{
    memset(app, 0, sizeof(*app));

    app->allocator = default_allocator;

    oph_app_init_presentation(config, app);

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
    const char **glfw_extensions = nullptr;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    create_info.enabledExtensionCount = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    create_info.enabledLayerCount = 0;

    VkResult res = vkCreateInstance(
        &create_info,
        nullptr,
        &app->backend.instance);
    if(res)
    {
        abort();
    }

    oph_app_init_vk_physical_devices(app);
    oph_app_init_vk_logical_devices(
        app,
        app->backend.physical_device,
        &app->backend.logical_device);
    oph_app_init_vk_swapchain(app);
}

void oph_app_destroy(struct oph_application *app)
{
    vkDestroyInstance(app->backend.instance, nullptr);
}