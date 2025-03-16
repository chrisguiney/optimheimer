//
// Created by chrisg on 3/15/25.
//

#ifndef OPH_H
#define OPH_H
#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "nuklear.h"

struct oph_frame
{
    int width;
    int height;
};

struct oph_display
{
    int width;
    int height;
};

struct oph_device
{
    int32_t font_id;
    int32_t font_width;
    int32_t font_height;

    struct nk_command_buffer cmds;
    struct nk_font_atlas atlas;
    struct nk_draw_null_texture null_texture;
};

struct oph_allocator;

typedef void* oph_allocator_ctx;
typedef void* (*oph_calloc_fn)(oph_allocator_ctx ctx, size_t nmemb, size_t size);
typedef void (*oph_free_fn)(oph_allocator_ctx ctx, void*);

struct oph_allocator
{
    oph_allocator_ctx ctx;
    oph_calloc_fn calloc;
    oph_free_fn free;
};


struct oph_vk_physical_device
{
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;

    uint32_t n_queue_families;
    VkQueueFamilyProperties* queue_families;

    bool have_graphics_queue_family;
    uint32_t graphics_queue_family;
};

struct oph_vk_logical_device
{
    VkDevice handle;
    VkQueue graphics_queue;
};

struct oph_vk_backend
{
    VkInstance instance;
    VkPhysicalDevice* vk_physical_devices;

    uint32_t n_physical_devices;
    struct oph_vk_physical_device* physical_devices;
    struct oph_vk_physical_device* physical_device;

    struct oph_vk_logical_device logical_device;

};

struct oph_application
{
    struct oph_allocator allocator;
    struct oph_frame frame;
    struct oph_display display;
    struct oph_device device;

    struct oph_vk_backend backend;
};


#endif //OPH_H
