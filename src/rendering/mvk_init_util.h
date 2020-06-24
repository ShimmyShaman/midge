/* mvk_init_util.h */

#ifndef MVK_INIT_UTIL_H
#define MVK_INIT_UTIL_H

#include <vector>

#define GLM_FORCE_RADIANS
#include <cglm/cglm.h>
#include <cglm/types-struct.h>
// #include "glm/glm.hpp"
// #include <glm/gtc/matrix_transform.hpp>

#include "rendering/xcbwindow.h"

/* Number of descriptor sets needs to be the same at alloc,       */
/* pipeline layout creation, and descriptor set layout creation   */
#define NUM_DESCRIPTOR_SETS 1
#define MAX_DESCRIPTOR_SETS 128

/* Number of viewports and number of scissors have to be the same */
/* at pipeline creation and in any call to set them dynamically   */
/* They also have to be the same as each other                    */
#define NUM_VIEWPORTS 1
#define NUM_SCISSORS NUM_VIEWPORTS

const uint RESOURCE_UID_BEGIN = 300;

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

/* Number of samples needs to be the same at image creation,
 * renderpass creation and pipeline creation.
 */
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT
#define VK_IMAGE_FORMAT VK_FORMAT_R8G8B8A8_SRGB;

typedef enum MVkResult {
  MVK_ERROR_GRAPHIC_QUEUE_NOT_FOUND = -1900,
  MVK_ERROR_PRESENT_QUEUE_NOT_FOUND,
  MVK_ERROR_UNSUPPORTED_DEPTH_FORMAT,
} MVkResult;

struct glsl_shader {
  const char *text;
  VkShaderStageFlagBits stage;
};

typedef struct queued_copy_info {
  void *p_source;
  VkDeviceSize dest_offset;
  size_t size_in_bytes;
} queued_copy_info;

typedef struct coloured_rect_draw_data {
  struct {
    // Vertex Fields
    vec2 offset;
    vec2 scale;
  } vert;

  struct {
    // Fragment Fields
    vec4 tint_color;
  } frag;

} coloured_rect_draw_data;

typedef struct sampled_image {
  VkFormat format;
  uint width, height;
  VkDeviceSize size;
  VkSampler sampler;
  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  VkFramebuffer framebuffer;
} sampled_image;

typedef struct textured_image_vertex {
  vec2 position;
  vec3 color;
  vec2 tex_coord;
} textured_image_vertex;

typedef struct render_program {
  VkDescriptorSetLayout desc_layout;
  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;
} render_program;

typedef struct loaded_font_info {
  const char *name;
  uint resource_uid;
} loaded_font_info;

/*
 * Keep each of our swap chain buffers' image, command buffer and view in one
 * spot
 */
typedef struct _swap_chain_buffers {
  VkImage image;
  VkImageView view;
} swap_chain_buffer;

typedef struct _vertex_input_description {
  VkVertexInputBindingDescription binding;
  VkVertexInputAttributeDescription attribs[2];
} vertex_input_description;

/*
 * A layer can expose extensions, keep track of those
 * extensions here.
 */
typedef struct layer_properties {
  VkLayerProperties properties;
  std::vector<VkExtensionProperties> instance_extensions;
  std::vector<VkExtensionProperties> device_extensions;
} layer_properties;

typedef struct vk_render_state {
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
  VkPhysicalDevice *gpus;
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
  uint32_t maximal_image_width, maximal_image_height;
  VkFormat format;

  uint32_t swapchainImageCount;
  VkSwapchainKHR swap_chain;
  std::vector<swap_chain_buffer> buffers;
  // VkSemaphore imageAcquiredSemaphore;

  struct {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDescriptorBufferInfo buffer_info;
    VkDeviceSize frame_utilized_amount;

    unsigned int queued_copies_alloc;
    unsigned int queued_copies_count;
    queued_copy_info *queued_copies;
  } render_data_buffer;

  struct {
    uint count;
    uint allocated;
    sampled_image *samples;
  } textures;

  struct {
    uint count;
    uint allocated;
    loaded_font_info *fonts;
  } loaded_fonts;

  render_program texture_prog;

  VkCommandPool cmd_pool;

  struct {
    VkFormat format;

    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
  } depth;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
  } global_vert_uniform_buffer;

  unsigned int ui_elements_allocated;
  unsigned int ui_element_index;
  coloured_rect_draw_data *ui_elements;

  struct {
    VkDescriptorImageInfo image_info;
  } texture_data;

  vertex_input_description pos_color_vertex_input_description;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
    vertex_input_description vi_desc;
  } cube_vertices;

  mat4s Projection;
  mat4s View;
  mat4s Model;
  mat4s Clip;
  mat4s MVP;

  // HEADLESS IMAGE
  struct {
    VkFormat format;

    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
  } headless;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
    vertex_input_description vi_desc;
  } shape_vertices;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
  } textured_shape_vertices;

  // Buffer for initialization commands
  VkCommandBuffer cmd;
  VkPipelineLayout pipeline_layout;
  std::vector<VkDescriptorSetLayout> desc_layout;
  VkPipelineCache pipelineCache;
  VkRenderPass present_render_pass, offscreen_render_pass;
  VkPipeline pipeline;

  VkPipelineShaderStageCreateInfo shaderStages[2];

  VkDescriptorPool desc_pool;
  unsigned int descriptor_sets_count;
  VkDescriptorSet descriptor_sets[MAX_DESCRIPTOR_SETS];

  uint32_t current_buffer;
  uint32_t queue_family_count;

  VkViewport viewport;
  VkRect2D scissor;
} vk_render_state;

VkResult mvk_init_global_layer_properties(vk_render_state *p_vkrs);
void mvk_init_device_extension_names(vk_render_state *p_vkrs);
// VkResult mvk_init_instance(vk_render_state *p_vkrs, char const *const app_short_name);
void mvk_init_instance(VkResult *res, vk_render_state *p_vkrs, char const *const app_short_name);
VkResult mvk_init_enumerate_device(vk_render_state *p_vkrs);
VkResult mvk_init_depth_buffer(vk_render_state *p_vkrs);
VkResult mvk_init_headless_image(vk_render_state *p_vkrs);
VkResult mvk_init_device(vk_render_state *p_vkrs);
// VkResult init_depth_buffer(vk_render_state *p_vkrs);
VkResult mvk_init_swapchain_extension(vk_render_state *p_vkrs);
VkResult mvk_init_swapchain(vk_render_state *p_vkrs);
VkResult mvk_init_uniform_buffer(vk_render_state *p_vkrs);
VkResult mvk_init_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs);
VkResult mvk_init_present_renderpass(vk_render_state *p_vkrs);
VkResult mvk_init_offscreen_renderpass(vk_render_state *p_vkrs);
VkResult mvk_init_command_pool(vk_render_state *p_vkrs);
VkResult mvk_init_command_buffer(vk_render_state *p_vkrs);
// VkResult mvk_execute_begin_command_buffer(vk_render_state *p_vkrs);
// VkResult mvk_execute_end_command_buffer(vk_render_state *p_vkrs);
VkResult mvk_execute_queue_command_buffer(vk_render_state *p_vkrs);
void mvk_init_device_queue(vk_render_state *p_vkrs);
VkResult mvk_init_shader(vk_render_state *p_vkrs, struct glsl_shader *glsl_shader, int stage_index);
VkResult mvk_init_textured_render_prog(vk_render_state *p_vkrs);
VkResult mvk_init_framebuffers(vk_render_state *p_vkrs, bool include_depth);
VkResult mvk_init_cube_vertices(vk_render_state *p_vkrs, const void *vertexData, uint32_t dataSize, uint32_t dataStride,
                                bool use_texture);
VkResult mvk_init_shape_vertices(vk_render_state *p_vkrs);
VkResult mvk_init_descriptor_pool(vk_render_state *p_vkrs, bool use_texture);
VkResult mvk_init_descriptor_set(vk_render_state *p_vkrs, bool use_texture);
VkResult mvk_init_pipeline_cache(vk_render_state *p_vkrs);
VkResult mvk_init_pipeline(vk_render_state *p_vkrs);
void mvk_init_viewports(vk_render_state *p_vkrs, unsigned int width, unsigned int height);
void mvk_init_scissors(vk_render_state *p_vkrs, unsigned int width, unsigned int height);

void mvk_destroy_pipeline(vk_render_state *p_vkrs);
void mvk_destroy_pipeline_cache(vk_render_state *p_vkrs);
void mvk_destroy_descriptor_pool(vk_render_state *p_vkrs);
void mvk_destroy_resources(vk_render_state *p_vkrs);
void mvk_destroy_framebuffers(vk_render_state *p_vkrs);
void mvk_destroy_shaders(vk_render_state *p_vkrs);
void mvk_destroy_textured_render_prog(vk_render_state *p_vkrs);
void mvk_destroy_uniform_buffer(vk_render_state *p_vkrs);
void mvk_destroy_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs);
void mvk_destroy_command_buffer(vk_render_state *p_vkrs);
void mvk_destroy_command_pool(vk_render_state *p_vkrs);
void mvk_destroy_headless_image(vk_render_state *p_vkrs);
void mvk_destroy_depth_buffer(vk_render_state *p_vkrs);
void mvk_destroy_swap_chain(vk_render_state *p_vkrs);
void mvk_destroy_renderpass(vk_render_state *p_vkrs);
void mvk_destroy_device(vk_render_state *p_vkrs);
void mvk_destroy_instance(vk_render_state *p_vkrs);

bool get_memory_type_index_from_properties(vk_render_state *p_vkrs, uint32_t typeBits, VkFlags requirements_mask,
                                           uint32_t *typeIndex);
VkResult createBuffer(vk_render_state *p_vkrs, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties,
                      VkBuffer *buffer, VkDeviceMemory *bufferMemory);
VkResult GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *p_shader_text, std::vector<unsigned int> &spirv);
#endif // MVK_INIT_UTIL_H