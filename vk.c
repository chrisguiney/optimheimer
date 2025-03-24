//
// Created by chrisg on 3/23/25.
//

#include "vk.h"

#include "allocator.h"
#include "oph.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool oph_vk_is_vk_device_suitable(struct oph_vk_state *vk,
                                         struct oph_vk_physical_device *dev)
{
    return dev->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && dev->features.geometryShader && dev->n_surface_formats > 0
        && dev->n_present_modes > 0;
}

static VkSurfaceFormatKHR
oph_vk_select_vk_surface_format(struct oph_vk_state *vk,
                                struct oph_vk_physical_device *dev)
{
    assert(dev->n_surface_formats > 0);
    for(size_t i = 0; i < dev->n_surface_formats; ++i)
    {
        VkSurfaceFormatKHR f = dev->surface_formats[i];
        if(f.format == VK_FORMAT_B8G8R8A8_SRGB
           && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return f;
        }
    }
    return dev->surface_formats[0];
}

static VkPresentModeKHR oph_vk_select_vk_swap_present_mode()
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

static VkExtent2D
oph_vk_get_vk_extent(struct oph_vk_state *vk,
                     const VkSurfaceCapabilitiesKHR *capabilities)
{
    if(capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }

    VkExtent2D extent
        = {.width = clamp_uint32(vk->presentation.framebuffer_width,
                                 capabilities->minImageExtent.width,
                                 capabilities->maxImageExtent.width),

           .height = clamp_uint32(vk->presentation.framebuffer_height,
                                  capabilities->minImageExtent.height,
                                  capabilities->maxImageExtent.height)};

    return extent;
}

static void oph_vk_init_dev_present_mode(struct oph_sys *sys,
                                         struct oph_vk_state *vk,
                                         struct oph_vk_physical_device *dev)
{
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        dev->handle, dev->surface, &dev->n_present_modes, nullptr);

    dev->present_modes
        = oph_calloc(sys, dev->n_present_modes, sizeof(*dev->present_modes));

    vkGetPhysicalDeviceSurfacePresentModesKHR(
        dev->handle, dev->surface, &dev->n_present_modes, dev->present_modes);
}

static void oph_vk_init_dev_surface(struct oph_sys *sys,
                                    struct oph_vk_state *vk,
                                    struct oph_vk_physical_device *dev)
{
    VkResult res = glfwCreateWindowSurface(
        vk->instance, vk->presentation.window, nullptr, &dev->surface);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        dev->handle, dev->surface, &dev->surface_capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
        dev->handle, dev->surface, &dev->n_surface_formats, nullptr);

    dev->surface_formats
        = oph_calloc(sys, dev->n_surface_formats, sizeof(VkSurfaceFormatKHR));

    vkGetPhysicalDeviceSurfaceFormatsKHR(dev->handle,
                                         dev->surface,
                                         &dev->n_surface_formats,
                                         dev->surface_formats);
}

static void oph_vk_init_dev_queue_families(struct oph_sys *sys,
                                           struct oph_vk_physical_device *dev)
{
    vkGetPhysicalDeviceQueueFamilyProperties(
        dev->handle, &dev->n_queue_families, nullptr);

    dev->queue_families
        = oph_calloc(sys, dev->n_queue_families, sizeof(*dev->queue_families));

    vkGetPhysicalDeviceQueueFamilyProperties(
        dev->handle, &dev->n_queue_families, dev->queue_families);

    for(size_t i = 0; i < dev->n_queue_families; ++i)
    {
        if(dev->queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            dev->graphics_queue_family = i;
            dev->have_graphics_queue_family = true;
        }

        vkGetPhysicalDeviceSurfaceSupportKHR(
            dev->handle, i, dev->surface, &dev->have_present_queue_family);
    }
}

static void oph_vk_init_dev_extensions(struct oph_sys *sys,
                                       struct oph_vk_state *vk,
                                       struct oph_vk_physical_device *dev)
{
    vkEnumerateDeviceExtensionProperties(
        dev->handle, nullptr, &dev->n_extension_properties, nullptr);

    dev->extension_properties = oph_calloc(
        sys, dev->n_extension_properties, sizeof(*dev->extension_properties));

    vkEnumerateDeviceExtensionProperties(dev->handle,
                                         nullptr,
                                         &dev->n_extension_properties,
                                         dev->extension_properties);
}

static void oph_vk_init_physical_device(struct oph_sys *sys,
                                        struct oph_vk_state *vk,
                                        VkPhysicalDevice vk_dev,
                                        struct oph_vk_physical_device *dev)
{
    assert(vk->presentation.window != nullptr);

    dev->handle = vk_dev;
    vkGetPhysicalDeviceProperties(dev->handle, &dev->properties);
    vkGetPhysicalDeviceFeatures(dev->handle, &dev->features);
    oph_vk_init_dev_surface(sys, vk, dev);
    oph_vk_init_dev_present_mode(sys, vk, dev);
    oph_vk_init_dev_extensions(sys, vk, dev);
    oph_vk_init_dev_queue_families(sys, dev);
}

static void oph_vk_init_physical_devices(struct oph_sys *sys,
                                         struct oph_vk_state *vk)
{
    vkEnumeratePhysicalDevices(
        vk->instance, &vk->backend.n_physical_devices, nullptr);

    vk->backend.vk_physical_devices
        = oph_calloc(sys,
                     vk->backend.n_physical_devices,
                     sizeof(*vk->backend.vk_physical_devices));

    vk->backend.physical_devices
        = oph_calloc(sys,
                     vk->backend.n_physical_devices,
                     sizeof(*vk->backend.physical_devices));

    vkEnumeratePhysicalDevices(vk->instance,
                               &vk->backend.n_physical_devices,
                               vk->backend.vk_physical_devices);

    for(size_t i = 0; i < vk->backend.n_physical_devices; ++i)
    {
        oph_vk_init_physical_device(sys,
                                    vk,
                                    vk->backend.vk_physical_devices[i],
                                    &vk->backend.physical_devices[i]);

        if(oph_vk_is_vk_device_suitable(vk, &vk->backend.physical_devices[i]))
        {
            vk->backend.physical_device = &vk->backend.physical_devices[i];
            break;
        }

        if(vk->backend.physical_device)
        {
            abort();
        }
    }
}

static const char *device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static void oph_vk_init_logical_devices(struct oph_vk_state *vk,
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

    device_create_info.enabledExtensionCount
        = sizeof(device_extensions) / sizeof(device_extensions[0]);
    device_create_info.ppEnabledExtensionNames = device_extensions;

    device_create_info.enabledLayerCount = 0;

    VkResult res = VK_SUCCESS;

    res = vkCreateDevice(
        dev->handle, &device_create_info, nullptr, &ldev->handle);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    vkGetDeviceQueue(
        ldev->handle, dev->graphics_queue_family, 0, &ldev->graphics_queue);
}

static void oph_vk_init_presentation(struct oph_vk_state *vk)
{
    vk->presentation.window
        = glfwCreateWindow(vk->defaults.initial_window_width,
                           vk->defaults.initial_window_height,
                           "Optimheimer",
                           nullptr,
                           nullptr);
    if(!vk->presentation.window)
    {
        abort();
    }

    glfwGetFramebufferSize(vk->presentation.window,
                           &vk->presentation.framebuffer_width,
                           &vk->presentation.framebuffer_height);
}

static void oph_vk_init_vk_imageview(struct oph_vk_logical_device *ldev,
                                     struct oph_vk_swapchain *swapchain,
                                     uint32_t i)
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
        ldev->handle, &create_info, nullptr, &swapchain->image_views[i]);

    if(res != VK_SUCCESS)
    {
        abort();
    }
}

static void oph_vk_init_swapchain(struct oph_sys *sys, struct oph_vk_state *vk)
{
    vk->swapchain.extent = oph_vk_get_vk_extent(
        vk, &vk->backend.physical_device->surface_capabilities);

    vk->swapchain.surface_format
        = oph_vk_select_vk_surface_format(vk, vk->backend.physical_device);

    vk->swapchain.present_mode = oph_vk_select_vk_swap_present_mode();

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vk->backend.physical_device->surface;
    create_info.minImageCount
        = vk->backend.physical_device->surface_capabilities.minImageCount;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const uint32_t queue_family_indices[]
        = {vk->backend.physical_device->graphics_queue_family,
           vk->backend.physical_device->present_queue_family};

    if(queue_family_indices[0] != queue_family_indices[1])
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 1;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform
        = vk->backend.physical_device->surface_capabilities.currentTransform;

    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = vk->swapchain.present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(vk->backend.logical_device.handle,
                                        &create_info,
                                        nullptr,
                                        &vk->swapchain.handle);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    res = vkGetSwapchainImagesKHR(vk->backend.logical_device.handle,
                                  vk->swapchain.handle,
                                  &vk->swapchain.n_images,
                                  nullptr);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    vk->swapchain.images = oph_calloc(
        sys, vk->swapchain.n_images, sizeof(*vk->swapchain.images));

    res = vkGetSwapchainImagesKHR(vk->backend.logical_device.handle,
                                  vk->swapchain.handle,
                                  &vk->swapchain.n_images,
                                  vk->swapchain.images);

    if(res != VK_SUCCESS)
    {
        abort();
    }

    vk->swapchain.image_views = oph_calloc(
        sys, vk->swapchain.n_images, sizeof(*vk->swapchain.image_views));

    for(size_t i = 0; i < vk->swapchain.n_images; ++i)
    {
        oph_vk_init_vk_imageview(
            &vk->backend.logical_device, &vk->swapchain, i);
    }
}

static int oph_vk_sql_scan_vertex_shaders_cb(void *ctx,
                                             int n_columns,
                                             char **column_names,
                                             char **result_column_names)
{
    struct oph_vk_state *vk = ctx;
    printf("in %s with %d columns\n", __FUNCTION__, n_columns);
    return 0;
}

static void oph_vk_init_shaders(struct oph_sys *sys, struct oph_vk_state *vk)
{
    char *errmsg = nullptr;
    sqlite3_exec(sys->db,
                 "select data from filesystem where name = 'shader.vert'",
                 oph_vk_sql_scan_vertex_shaders_cb,
                 vk,
                 &errmsg);
}

void oph_vk_init(struct oph_sys *sys,
                 const struct oph_vk_defaults *defaults,
                 struct oph_vk_state *vk)
{
    memset(vk, 0, sizeof(*vk));
    vk->defaults = *defaults;

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

    VkResult res = vkCreateInstance(&create_info, nullptr, &vk->instance);
    if(res)
    {
        abort();
    }

    oph_vk_init_presentation(vk);
    oph_vk_init_physical_devices(sys, vk);
    oph_vk_init_logical_devices(
        vk, vk->backend.physical_device, &vk->backend.logical_device);
    oph_vk_init_swapchain(sys, vk);
    oph_vk_init_shaders(sys, vk);
}
