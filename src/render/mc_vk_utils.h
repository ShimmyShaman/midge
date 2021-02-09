/* render_thread.h */

#ifndef MC_VK_UTILS_H
#define MC_VK_UTILS_H

#include "render/mc_vulkan.h"

VkResult mvk_init_resources(vk_render_state *p_vkrs);
void mvk_destroy_resources(vk_render_state *p_vkrs);
VkResult mvk_init_shape_vertices(vk_render_state *p_vkrs);
VkResult mvk_load_image_sampler(vk_render_state *p_vkrs, const int texWidth, const int texHeight, const int texChannels,
                                mvk_image_sampler_usage image_usage, const unsigned char *const pixels,
                                mcr_texture_image **out_image);
VkResult mvk_load_texture_from_file(vk_render_state *p_vkrs, const char *const filepath,
                                    mcr_texture_image **p_resource);
VkResult mvk_create_empty_render_target(vk_render_state *p_vkrs, const uint width, const uint height,
                                        mvk_image_sampler_usage image_usage, mcr_texture_image **p_resource);
VkResult mvk_load_font(vk_render_state *p_vkrs, const char *const filepath, float font_height,
                       mcr_font_resource **p_resource);
VkResult mvk_load_vertex_data_buffer(vk_render_state *p_vkrs, float *vertices, unsigned int vertex_count,
                                     bool release_original_data_on_copy, mcr_vertex_buffer **p_resource);
VkResult mvk_load_index_buffer(vk_render_state *p_vkrs, unsigned int *p_data, unsigned int data_count,
                               bool release_original_data_on_copy, mcr_index_buffer **p_resource);
VkResult mvk_remap_mem(vk_render_state *p_vkrs, VkDeviceMemory mem, VkDeviceSize offset, VkDeviceSize range, void *data,
                       size_t data_size_in_bytes);
VkResult mvk_create_render_program(vk_render_state *p_vkrs, mcr_render_program_create_info *create_info,
                                   mcr_render_program **out_render_program);

#endif // MC_VK_UTILS_H