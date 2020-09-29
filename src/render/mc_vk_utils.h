/* render_thread.h */

#ifndef MC_VK_UTILS_H
#define MC_VK_UTILS_H

#include "midge_common.h"
#include "render/mc_vulkan.h"

extern "C" {

VkResult mvk_init_resources(vk_render_state *p_vkrs);
void mvk_destroy_resources(vk_render_state *p_vkrs);
VkResult mvk_init_shape_vertices(vk_render_state *p_vkrs);
VkResult mvk_load_image_sampler(vk_render_state *p_vkrs, const int texWidth, const int texHeight, const int texChannels,
                                bool use_as_render_target, const unsigned char *const pixels,
                                texture_image *image_sampler);
VkResult mvk_load_texture_from_file(vk_render_state *p_vkrs, const char *const filepath, uint *resource_uid);
VkResult mvk_create_empty_render_target(vk_render_state *p_vkrs, const uint width, const uint height,
                                        bool use_as_render_target, uint *resource_uid);
VkResult mvk_load_font(vk_render_state *p_vkrs, const char *const filepath, float font_height, uint *resource_uid);
VkResult mvk_load_mesh(vk_render_state *p_vkrs, float *vertices, unsigned int vertex_count, uint *resource_uid);
}

#endif // MC_VK_UTILS_H