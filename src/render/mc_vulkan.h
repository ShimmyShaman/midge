/* mc_vulkan.h */

#ifndef MC_VULKAN_H
#define MC_VULKAN_H

#include "platform/mc_xcb.h"
#include "render/render_common.h"
#include <vulkan/vulkan_core.h>

#define MAX_DESCRIPTOR_SETS 4096
#define VK_IMAGE_FORMAT VK_FORMAT_R8G8B8A8_SRGB;

/* Amount of time, in nanoseconds, to wait for a command buffer to complete */
#define FENCE_TIMEOUT 100000000

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

typedef struct textured_image_vertex {
  vec2 position;
  vec2 tex_coord;
} textured_image_vertex;

typedef struct mcr_render_program {
  unsigned int resource_uid;
  VkDescriptorSetLayout descriptor_layout;
  VkPipelineLayout pipeline_layout;
  VkPipeline pipeline;
} mcr_render_program;

typedef struct font_resource {
  const char *name;
  float height;
  float draw_vertical_offset;
  mcr_texture_image *texture;
  stbtt_bakedchar *char_data;
} font_resource;

typedef struct loaded_font_list {
  uint32_t count;
  uint32_t capacity;
  font_resource **fonts;
} loaded_font_list;

typedef struct mcr_vertex_buffer {
  unsigned int resource_uid;
  VkBuffer buf;
  VkDeviceMemory mem;
  VkDescriptorBufferInfo buffer_info;
} mcr_vertex_buffer;

typedef struct mcr_index_buffer {
  unsigned int resource_uid;
  unsigned int count;
  VkBuffer buf;
  VkDeviceMemory mem;
  VkDescriptorBufferInfo buffer_info;
} mcr_index_buffer;

typedef struct mcr_render_program_data {
  void **input_buffers;
  mcr_index_buffer *indices;
  mcr_vertex_buffer *vertices;

} mcr_render_program_data;

typedef struct mcr_texture_image {
  unsigned int resource_uid;
  mvk_image_sampler_usage sampler_usage;
  VkFormat format;
  uint32_t width, height;
  VkDeviceSize size;
  VkSampler sampler;
  VkImage image;
  VkDeviceMemory memory;
  VkImageView view;
  VkFramebuffer framebuffer;
} mcr_texture_image;

typedef struct mvk_dynamic_buffer_block {
  VkDeviceSize allocated_size;
  VkDeviceMemory memory;

  VkBuffer buffer;
  VkDeviceSize utilized;
} mvk_dynamic_buffer_block;

typedef struct vk_render_state {

  unsigned int resource_uid_counter;
  unsigned int presentation_updates;

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
    uint32_t size_count;
    VkExtent2D extents;
    VkCommandBuffer *command_buffers;
    VkImage *images;
    VkImageView *image_views;
    VkFramebuffer *framebuffers;
  } swap_chain;

  struct {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkFormat format;
  } depth_buffer;

  // HEADLESS IMAGE
  struct {
    VkFormat format;

    VkImage image;
    VkCommandBuffer command_buffer;
    VkDeviceMemory memory;
    VkImageView view;
  } headless;

  mat4s Projection;
  mat4s View;
  mat4s Model;
  mat4s Clip;
  mat4s MVP;

  // struct {
  //   VkBuffer buf;
  //   VkDeviceMemory mem;
  //   VkDescriptorBufferInfo buffer_info;
  // } global_vert_uniform_buffer;

  struct {
    struct {
      VkDeviceSize min_memory_allocation;

      unsigned int size;
      unsigned int activated;
      mvk_dynamic_buffer_block **blocks;
    } dynamic_buffers;

    // VkDeviceSize allocated_size;
    // VkBuffer buffer;
    // VkDeviceSize buffer_offset;
    // VkDeviceMemory memory;
    // VkDeviceSize frame_utilized_amount;

    unsigned int queued_copies_alloc;
    unsigned int queued_copies_count;
    queued_copy_info *queued_copies;
  } render_data_buffer;

  VkRenderPass present_render_pass, offscreen_render_pass_2d, offscreen_render_pass_3d;

  mcr_render_program tint_prog, texture_prog, font_prog, mesh_prog;
  VkPipelineCache pipelineCache;

  VkDescriptorPool descriptor_pool;
  unsigned int descriptor_sets_count;
  VkDescriptorSet descriptor_sets[MAX_DESCRIPTOR_SETS];

  /*******************
   *     Resources   *
   *******************/
  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
  } shape_vertices;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
  } textured_shape_vertices;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
  } cube_shape_vertices;

  struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    VkDescriptorBufferInfo buffer_info;
  } cube_shape_indices;

  // Loaded Indexed Resources
  struct {
    uint32_t count, alloc;
    mcr_texture_image **items;
  } textures;
  loaded_font_list loaded_fonts;
  struct {
    uint32_t count, alloc;
    mcr_vertex_buffer **items;
  } loaded_vertex_data;
  struct {
    uint32_t count, alloc;
    mcr_index_buffer **items;
  } loaded_index_data;
  struct {
    uint32_t count, alloc; // TODO rename alloc >> capacity
    mcr_render_program **items;
  } loaded_render_programs;
} vk_render_state;

extern "C" {
VkResult mvk_init_vulkan(vk_render_state *vkrs);

bool mvk_get_properties_memory_type_index(vk_render_state *p_vkrs, uint32_t typeBits, VkFlags requirements_mask,
                                          uint32_t *typeIndex);
}

#endif // MC_VULKAN_H