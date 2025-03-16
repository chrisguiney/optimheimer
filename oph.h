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

struct oph_device {
    int32_t font_id;
    int32_t font_width;
    int32_t font_height;

    struct nk_command_buffer cmds;
    struct nk_font_atlas atlas;
    struct nk_draw_null_texture null_texture;
};

struct oph_application
{
    struct oph_frame frame;
    struct oph_display display;
    struct oph_device device;

    struct
    {
        VkInstance instance;
    } backend;
};


#endif //OPH_H
