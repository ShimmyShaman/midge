/* renderer.h */

#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "rendering/mvk_core.h"
#include "rendering/xcbwindow.h"

typedef struct
{
    VkInstance instance;
    VkDevice device;
    VkQueue queue;
    VkSurfaceKHR surface;

    VkPhysicalDevice gpu;
    VkPhysicalDeviceProperties gpu_properties;
    uint32_t graphics_family_index;
} vk_render_state;

VkResult initVulkan(vk_render_state *p_vkrs, mxcb_window_info *p_wnfo);
VkResult initDevice(vk_render_state *p_vkstate);
void deInitVulkan(vk_render_state *p_vkstate);

#endif // RENDERER_H