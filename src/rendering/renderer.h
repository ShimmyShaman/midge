/* renderer.h */

#ifndef RENDERER_H
#define RENDERER_H

#include "m_threads.h"

#include "rendering/mvk_init_util.h"
#include "rendering/xcbwindow.h"

// VkResult initVulkan(vk_render_state *p_vkrs, mxcb_window_info *p_wnfo);
VkResult initDevice(vk_render_state *p_vkstate);
void deInitVulkan(vk_render_state *p_vkstate);

#endif // RENDERER_H