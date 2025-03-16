#include <string.h>
#include <GL/gl.h>

#include "nuklear.h"
#include "device.h"


void oph_device_init(struct oph_device* dev)
{
    memset(dev, 0, sizeof(*dev));

    nk_font_atlas_init_default(&dev->atlas);
    nk_font_atlas_begin(&dev->atlas);

    const void* image = nk_font_atlas_bake(&dev->atlas, &dev->font_width, &dev->font_height, NK_FONT_ATLAS_RGBA32);

    glGenTextures(1, (GLuint*)(&dev->font_id));
    glBindTexture(GL_TEXTURE_2D, dev->font_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dev->font_width, dev->font_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    nk_font_atlas_end(&dev->atlas, nk_handle_id(dev->font_id), &dev->null_texture);
}


void oph_device_draw(struct oph_device* dev)
{
}