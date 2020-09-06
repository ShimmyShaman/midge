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
  const char **device_extension_names;
  struct {
    uint32_t size;
    const char **items;
  } instance_layer_names;

} vk_render_state;

extern "C" {
VkResult mvk_init_vulkan(vk_render_state *vkrs);
}

#endif // MC_VULKAN_H