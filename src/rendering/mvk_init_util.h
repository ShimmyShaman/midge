/* mvk_init_util.h */

#ifndef MVK_INIT_UTIL_H
#define MVK_INIT_UTIL_H

#include <vector>

#include "rendering/xcbwindow.h"

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

/* Number of samples needs to be the same at image creation,
 * renderpass creation and pipeline creation.
 */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT

typedef enum MVkResult
{
    MVK_ERROR_GRAPHIC_QUEUE_NOT_FOUND = -1900,
    MVK_ERROR_PRESENT_QUEUE_NOT_FOUND,
    MVK_ERROR_UNSUPPORTED_DEPTH_FORMAT,
} MVkResult;

/*
 * Keep each of our swap chain buffers' image, command buffer and view in one
 * spot
 */
typedef struct _swap_chain_buffers
{
    VkImage image;
    VkImageView view;
} swap_chain_buffer;

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
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    uint32_t graphics_queue_family_index;
    uint32_t present_queue_family_index;
    VkPhysicalDeviceProperties gpu_props;
    std::vector<VkQueueFamilyProperties> queue_props;
    VkPhysicalDeviceMemoryProperties memory_properties;

    // VkFramebuffer *framebuffers;
    mxcb_window_info *xcb_winfo;
    uint32_t window_width, window_height;
    VkFormat format;

    uint32_t swapchainImageCount;
    VkSwapchainKHR swap_chain;
    std::vector<swap_chain_buffer> buffers;
    // VkSemaphore imageAcquiredSemaphore;

    VkCommandPool cmd_pool;

    struct
    {
        VkFormat format;

        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;

    // Buffer for initialization commands
    VkCommandBuffer cmd;
    // VkPipelineLayout pipeline_layout;
    // std::vector<VkDescriptorSetLayout> desc_layout;
    // VkPipelineCache pipelineCache;
    // VkRenderPass render_pass;
    // VkPipeline pipeline;

    uint32_t current_buffer;
    uint32_t queue_family_count;
} vk_render_state;

VkResult mvk_init_global_layer_properties(std::vector<layer_properties> *p_vk_layers);
void mvk_init_device_extension_names(vk_render_state *p_vkrs);
VkResult mvk_init_instance(vk_render_state *p_vkrs, char const *const app_short_name);
VkResult mvk_init_enumerate_device(vk_render_state *p_vkrs, uint32_t required_gpu_count);
VkResult mvk_init_depth_buffer(vk_render_state *p_vkrs);
VkResult mvk_init_device(vk_render_state *p_vkrs);
VkResult init_depth_buffer(vk_render_state *p_vkrs);
VkResult mvk_init_swapchain_extension(vk_render_state *p_vkrs);
VkResult mvk_init_swapchain(vk_render_state *p_vkrs, VkImageUsageFlags default_image_usage_flags);
VkResult mvk_init_command_pool(vk_render_state *p_vkrs);
VkResult mvk_init_command_buffer(vk_render_state *p_vkrs);
VkResult mvk_execute_begin_command_buffer(vk_render_state *p_vkrs);
VkResult mvk_execute_end_command_buffer(vk_render_state *p_vkrs);
VkResult mvk_execute_queue_command_buffer(vk_render_state *p_vkrs);
void mvk_init_device_queue(vk_render_state *p_vkrs);

void mvk_destroy_command_buffer(vk_render_state *p_vkrs);
void mvk_destroy_command_pool(vk_render_state *p_vkrs);
void mvk_destroy_depth_buffer(vk_render_state *p_vkrs);
void mvk_destroy_swap_chain(vk_render_state *p_vkrs);
void mvk_destroy_device(vk_render_state *p_vkrs);
void mvk_destroy_instance(vk_render_state *p_vkrs);

#endif // MVK_INIT_UTIL_H