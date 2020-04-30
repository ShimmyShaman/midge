/* mvk_init_util.h */

#ifndef MVK_INIT_UTIL_H
#define MVK_INIT_UTIL_H

#include <vector>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/xcbwindow.h"

/* Number of descriptor sets needs to be the same at alloc,       */
/* pipeline layout creation, and descriptor set layout creation   */
#define NUM_DESCRIPTOR_SETS 1

/* Number of viewports and number of scissors have to be the same */
/* at pipeline creation and in any call to set them dynamically   */
/* They also have to be the same as each other                    */
#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

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

struct glsl_shader
{
    const char *text;
    VkShaderStageFlagBits stage;
};

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
typedef struct layer_properties
{
    VkLayerProperties properties;
    std::vector<VkExtensionProperties> instance_extensions;
    std::vector<VkExtensionProperties> device_extensions;
} layer_properties;

typedef struct vk_render_state
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

    VkFramebuffer *framebuffers;
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

    struct
    {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo buffer_info;
    } uniform_data;

    struct
    {
        VkDescriptorImageInfo image_info;
    } texture_data;

    struct
    {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkDescriptorBufferInfo buffer_info;
    } vertex_buffer;
    VkVertexInputBindingDescription vi_binding;
    VkVertexInputAttributeDescription vi_attribs[2];

    glm::mat4 Projection;
    glm::mat4 View;
    glm::mat4 Model;
    glm::mat4 Clip;
    glm::mat4 MVP;

    // Buffer for initialization commands
    VkCommandBuffer cmd;
    VkPipelineLayout pipeline_layout;
    std::vector<VkDescriptorSetLayout> desc_layout;
    VkPipelineCache pipelineCache;
    VkRenderPass render_pass;
    VkPipeline pipeline;

    VkPipelineShaderStageCreateInfo shaderStages[2];

    VkDescriptorPool desc_pool;
    std::vector<VkDescriptorSet> desc_set;

    uint32_t current_buffer;
    uint32_t queue_family_count;

    VkViewport viewport;
    VkRect2D scissor;
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
VkResult mvk_init_uniform_buffer(vk_render_state *p_vkrs);
VkResult mvk_init_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs, bool use_texture, VkDescriptorSetLayoutCreateFlags descSetLayoutCreateFlags);
VkResult mvk_init_renderpass(vk_render_state *p_vkrs, bool include_depth, bool clear, VkImageLayout finalLayout, VkImageLayout initialLayout);
VkResult mvk_init_command_pool(vk_render_state *p_vkrs);
VkResult mvk_init_command_buffer(vk_render_state *p_vkrs);
VkResult mvk_execute_begin_command_buffer(vk_render_state *p_vkrs);
VkResult mvk_execute_end_command_buffer(vk_render_state *p_vkrs);
VkResult mvk_execute_queue_command_buffer(vk_render_state *p_vkrs);
void mvk_init_device_queue(vk_render_state *p_vkrs);
VkResult mvk_init_shader(vk_render_state *p_vkrs, struct glsl_shader *glsl_shader, int stage_index);
VkResult mvk_init_framebuffers(vk_render_state *p_vkrs, bool include_depth);
VkResult mvk_init_vertex_buffer(vk_render_state *p_vkrs, const void *vertexData, uint32_t dataSize, uint32_t dataStride, bool use_texture);
VkResult mvk_init_descriptor_pool(vk_render_state *p_vkrs, bool use_texture);
VkResult mvk_init_descriptor_set(vk_render_state *p_vkrs, bool use_texture);
VkResult mvk_init_pipeline_cache(vk_render_state *p_vkrs);
VkResult mvk_init_pipeline(vk_render_state *p_vkrs, VkBool32 include_depth, VkBool32 include_vi);
void mvk_init_viewports(vk_render_state *p_vkrs);
void mvk_init_scissors(vk_render_state *p_vkrs);

void mvk_destroy_pipeline(vk_render_state *p_vkrs);
void mvk_destroy_pipeline_cache(vk_render_state *p_vkrs);
void mvk_destroy_descriptor_pool(vk_render_state *p_vkrs);
void mvk_destroy_vertex_buffer(vk_render_state *p_vkrs);
void mvk_destroy_framebuffers(vk_render_state *p_vkrs);
void mvk_destroy_shaders(vk_render_state *p_vkrs);
void mvk_destroy_uniform_buffer(vk_render_state *p_vkrs);
void mvk_destroy_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs);
void mvk_destroy_command_buffer(vk_render_state *p_vkrs);
void mvk_destroy_command_pool(vk_render_state *p_vkrs);
void mvk_destroy_depth_buffer(vk_render_state *p_vkrs);
void mvk_destroy_swap_chain(vk_render_state *p_vkrs);
void mvk_destroy_renderpass(vk_render_state *p_vkrs);
void mvk_destroy_device(vk_render_state *p_vkrs);
void mvk_destroy_instance(vk_render_state *p_vkrs);

#endif // MVK_INIT_UTIL_H