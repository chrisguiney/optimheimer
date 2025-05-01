//
// Created by chrisg on 3/15/25.
//

#ifndef OPH_H
#define OPH_H
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "nuklear.h"

#include <sqlite3.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

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

typedef void *oph_allocator_ctx;
typedef void *(*oph_calloc_fn)(oph_allocator_ctx ctx,
                               size_t nmemb,
                               size_t size);
typedef void (*oph_free_fn)(oph_allocator_ctx ctx, void *);

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

    uint32_t n_extension_properties;
    VkExtensionProperties *extension_properties;

    uint32_t n_queue_families;
    VkQueueFamilyProperties *queue_families;

    VkBool32 have_graphics_queue_family;
    uint32_t graphics_queue_family;

    VkBool32 have_present_queue_family;
    uint32_t present_queue_family;

    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR surface_capabilities;

    uint32_t n_surface_formats;
    VkSurfaceFormatKHR *surface_formats;

    uint32_t n_present_modes;
    VkPresentModeKHR *present_modes;
};

struct oph_vk_logical_device
{
    VkDevice handle;
    VkQueue graphics_queue;
};

struct oph_vk_devices
{
    VkPhysicalDevice *vk_physical_devices;

    uint32_t n_physical_devices;
    struct oph_vk_physical_device *physical_devices;
    struct oph_vk_physical_device *physical_device;

    struct oph_vk_logical_device logical_device;
};

struct oph_vk_presentation
{
    GLFWwindow *window;
    int32_t framebuffer_width;
    int32_t framebuffer_height;
};

struct oph_vk_swapchain
{
    VkSwapchainKHR handle;
    VkSurfaceFormatKHR surface_format;
    VkPresentModeKHR present_mode;
    VkExtent2D extent;

    uint32_t n_images;
    VkImage *images;
    VkImageView *image_views;
};

struct oph_vk_config
{
    bool enable_validation_layers;
    int32_t initial_window_width;
    int32_t initial_window_height;
};

struct oph_vk_validation
{
    bool enable;
    uint32_t n_layers;
    VkLayerProperties *available_layers;
};

struct oph_vk_state
{
    VkInstance instance;
    struct oph_vk_config config;
    struct oph_vk_validation validation;
    struct oph_vk_devices backend;
    struct oph_vk_presentation presentation;
    struct oph_vk_swapchain swapchain;
};

struct oph_sys
{
    struct oph_allocator allocator;
    sqlite3 *db;
};

struct oph_application
{
    struct oph_sys sys;
    struct oph_frame frame;
    struct oph_display display;
    struct oph_device device;
    struct oph_vk_state vk;
};

struct oph_sys_config
{
    const char *db_path;
};

struct oph_app_config
{
    int32_t initial_window_width;
    int32_t initial_window_height;
};

struct oph_configuration
{
    struct oph_sys_config sys;
    struct oph_vk_config vk;
    struct oph_app_config app;

    bool enable_validation_layers;
};

#endif //OPH_H
