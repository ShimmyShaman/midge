/* mc_vulkan.h */

#ifndef MC_VULKAN_H
#define MC_VULKAN_H

#include "platform/mc_xcb.h"
#include <vulkan/vulkan_core.h>

/*
 * A layer can expose extensions, keep track of those
 * extensions here.
 */
typedef struct layer_properties {
  VkLayerProperties properties;
  struct {
    uint32_t size;
    VkExtensionProperties *items;
  } instance_extensions;
  struct {
    uint32_t size;
    VkExtensionProperties *items;
  } device_extensions;

} layer_properties;

typedef struct queued_copy_info {
  void *p_source;
  VkDeviceSize dest_offset;
  size_t size_in_bytes;
} queued_copy_info;

typedef struct vk_render_state {

  mxcb_window_info *xcb_winfo;
  uint32_t window_width, window_height;
  uint32_t maximal_image_width, maximal_image_height;

  struct {
    uint32_t size;
    layer_properties **items;
  } instance_layer_properties;
  struct {
    uint32_t size;
    const char **items;
  } device_extension_names;
  struct {
    uint32_t size;
    const char **items;
  } instance_layer_names;
  struct {
    uint32_t size;
    const char **items;
  } instance_extension_names;

  VkInstance instance;

  uint32_t device_count;
  VkPhysicalDevice *gpus;
  VkPhysicalDeviceProperties gpu_props;
  uint32_t queue_family_count;
  VkQueueFamilyProperties *queue_family_properties;
  VkPhysicalDeviceMemoryProperties memory_properties;

  VkSurfaceKHR surface;
  VkFormat format;
  uint32_t graphics_queue_family_index;
  uint32_t present_queue_family_index;
  VkDevice device;
  VkQueue graphics_queue;
  VkQueue present_queue;

  VkCommandPool command_pool;

  struct {
    VkSwapchainKHR instance;
    uint32_t current_index;
    uint32_t size;
    VkCommandBuffer *command_buffers;
    VkImage *images;
    VkImageView *image_views;
  } swap_chain;

  // HEADLESS IMAGE
  struct {
    VkFormat format;

    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
  } headless;

  mat4s Projection;
  mat4s View;
  mat4s Model;
  mat4s Clip;
  mat4s MVP;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
  } global_vert_uniform_buffer;

  struct {
    VkDeviceSize allocated_size;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize frame_utilized_amount;

    unsigned int queued_copies_alloc;
    unsigned int queued_copies_count;
    queued_copy_info *queued_copies;
  } render_data_buffer;

  VkPipelineLayout pipeline_layout;
  VkDescriptorSetLayout descriptor_layout;
  VkRenderPass present_render_pass, offscreen_render_pass;

  struct {
    VkDescriptorSetLayout descriptor_layout;
  } texture_prog;

} vk_render_state;

extern "C" {
VkResult mvk_init_vulkan(vk_render_state *vkrs);
}

#endif // MC_VULKAN_H