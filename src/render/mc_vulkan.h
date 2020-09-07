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

} vk_render_state;

extern "C" {
VkResult mvk_init_vulkan(vk_render_state *vkrs);
}

#endif // MC_VULKAN_H