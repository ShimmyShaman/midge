/* mvk_init_util.h */

#ifndef MVK_INIT_UTIL_H
#define MVK_INIT_UTIL_H

#include <vector>

#include "rendering/xcbwindow.h"

typedef enum MVkResult
{
    MVK_GRAPHIC_QUEUE_NOT_FOUND = -1700,
    MVK_PRESENT_QUEUE_NOT_FOUND = -1700,
} MVkResult;

/*
 * A layer can expose extensions, keep track of those
 * extensions here.
 */
typedef struct
{
    VkLayerProperties properties;
    std::vector<VkExtensionProperties> instance_extensions;
    std::vector<VkExtensionProperties> device_extensions;
} layer_properties;

typedef struct
{
    VkSurfaceKHR surface;
    // bool prepared;
    // bool use_staging_buffer;
    // bool save_images;

    std::vector<const char *> instance_layer_names;
    std::vector<const char *> instance_extension_names;
    std::vector<layer_properties> instance_layer_properties;
    std::vector<VkExtensionProperties> instance_extension_properties;
    VkInstance inst;

    std::vector<const char *> device_extension_names;
    // std::vector<VkExtensionProperties> device_extension_properties;
    std::vector<VkPhysicalDevice> gpus;
    // VkDevice device;
    // VkQueue graphics_queue;
    // VkQueue present_queue;
    uint32_t graphics_queue_family_index;
    uint32_t present_queue_family_index;
    VkPhysicalDeviceProperties gpu_props;
    std::vector<VkQueueFamilyProperties> queue_props;
    VkPhysicalDeviceMemoryProperties memory_properties;

    // VkFramebuffer *framebuffers;
    mxcb_window_info *xcb_winfo;
    uint32_t window_width, window_height;
    VkFormat format;

    uint32_t queue_family_count;
} vk_render_state;

VkResult mvk_init_global_layer_properties(std::vector<layer_properties> *p_vk_layers);
void init_device_extension_names(vk_render_state *p_vkrs);
VkResult mvk_init_instance(vk_render_state *p_vkrs, char const *const app_short_name);
VkResult init_enumerate_device(vk_render_state *p_vkrs, uint32_t required_gpu_count);
VkResult init_swapchain_extension(vk_render_state *p_vkrs);

#endif // MVK_INIT_UTIL_H