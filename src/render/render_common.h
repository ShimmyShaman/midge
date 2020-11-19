
/* render_thread.h */

#ifndef MC_RENDER_COMMON_H
#define MC_RENDER_COMMON_H

#include "platform/mc_xcb.h"

// TEMP will have to do function defines sooner or later
#define VK_CHECK(res, func_name)                               \
  if (res) {                                                   \
    printf("VK-ERR[%i] :%i --" func_name "\n", res, __LINE__); \
    return res;                                                \
  }
#define VK_ASSERT(condition, message)                        \
  if (!(condition)) {                                        \
    printf("VK-ASSERT ERR[" #condition "] :" #message "\n"); \
    return -11111;                                           \
  }

typedef struct render_color {
  float r, g, b, a;
} render_color;

#define COLOR_TRANSPARENT  \
  /*(render_color)*/ {     \
    0.0f, 0.0f, 0.0f, 0.0f \
  }
#define COLOR_CORNFLOWER_BLUE \
  /*(render_color)*/ {        \
    0.19f, 0.34f, 0.83f, 1.f  \
  }
#define COLOR_MIDNIGHT_EXPRESS \
  /*(render_color)*/ {         \
    0.10f, 0.10f, 0.17f, 1.f   \
  }
#define COLOR_DEEP_FIR       \
  /*(render_color)*/ {       \
    0.05f, 0.14f, 0.09f, 1.f \
  }
#define COLOR_MACARONI_AND_CHEESE \
  /*(render_color)*/ {            \
    1.f, 0.72f, 0.50f, 1.f        \
  }
#define COLOR_DARK_GREEN   \
  /*(render_color)*/ {     \
    0.0f, 0.25f, 0.0f, 1.f \
  }
#define COLOR_GREEN       \
  /*(render_color)*/ {    \
    0.0f, 0.5f, 0.0f, 1.f \
  }
#define COLOR_LIME       \
  /*(render_color)*/ {   \
    0.0f, 1.f, 0.0f, 1.f \
  }
#define COLOR_FUNCTION_GREEN                       \
  /*(render_color)*/ {                             \
    40.f / 255.f, 235.f / 255.f, 40.f / 255.f, 1.f \
  }
#define COLOR_FUNCTION_RED                          \
  /*(render_color)*/ {                              \
    215.f / 255.f, 195.f / 255.f, 40.f / 255.f, 1.f \
  }
#define COLOR_POWDER_BLUE                            \
  /*(render_color)*/ {                               \
    176.f / 255.f, 224.f / 255.f, 230.f / 255.f, 1.f \
  }
#define COLOR_LIGHT_SKY_BLUE                         \
  /*(render_color)*/ {                               \
    135.f / 255.f, 206.f / 255.f, 255.f / 250.f, 1.f \
  }
#define COLOR_NODE_ORANGE                           \
  /*(render_color)*/ {                              \
    216.f / 255.f, 134.f / 255.f, 51.f / 250.f, 1.f \
  }
#define COLOR_TEAL          \
  /*(render_color)*/ {      \
    0.0f, 0.52f, 0.52f, 1.f \
  }
#define COLOR_PURPLE                                \
  /*(render_color)*/ {                              \
    160.f / 255.f, 32.f / 255.f, 240.f / 255.f, 1.f \
  }
#define COLOR_BURLY_WOOD     \
  /*(render_color)*/ {       \
    0.87f, 0.72f, 0.52f, 1.f \
  }
#define COLOR_DARK_SLATE_GRAY \
  /*(render_color)*/ {        \
    0.18f, 0.18f, 0.31f, 1.f  \
  }
#define COLOR_NEARLY_BLACK   \
  /*(render_color)*/ {       \
    0.13f, 0.13f, 0.13f, 1.f \
  }
#define COLOR_GHOST_WHITE  \
  /*(render_color)*/ {     \
    0.97f, 0.97f, 1.f, 1.f \
  }
#define COLOR_BLACK    \
  /*(render_color)*/ { \
    0.f, 0.f, 0.f, 1.f \
  }
#define COLOR_GRAY        \
  /*(render_color)*/ {    \
    0.5f, 0.5f, 0.5f, 1.f \
  }
#define COLOR_DIM_GRAY    \
  /*(render_color)*/ {    \
    0.3f, 0.3f, 0.3f, 1.f \
  }
#define COLOR_DARK_GRAY   \
  /*(render_color)*/ {    \
    0.2f, 0.2f, 0.2f, 1.f \
  }
#define COLOR_YELLOW   \
  /*(render_color)*/ { \
    1.f, 1.f, 0.f, 1.f \
  }
#define COLOR_RED      \
  /*(render_color)*/ { \
    1.f, 0.f, 0.f, 1.f \
  }
#define COLOR_LIGHT_YELLOW       \
  /*(render_color)*/ {           \
    1.f, 1.f, 102.f / 255.f, 1.f \
  }

typedef enum element_render_command_type {
  RENDER_COMMAND_NONE = 1,
  RENDER_COMMAND_COLORED_QUAD,
  RENDER_COMMAND_TEXTURED_QUAD,
  RENDER_COMMAND_PRINT_TEXT,
  // RENDER_COMMAND_INDEXED_MESH,
  RENDER_COMMAND_PROGRAM,
} element_render_command_type;

typedef enum resource_command_type {
  RESOURCE_COMMAND_NONE = 1,
  RESOURCE_COMMAND_LOAD_TEXTURE,
  RESOURCE_COMMAND_CREATE_TEXTURE,
  RESOURCE_COMMAND_LOAD_FONT,
  RESOURCE_COMMAND_LOAD_VERTEX_BUFFER,
  RESOURCE_COMMAND_LOAD_INDEX_BUFFER,
  RESOURCE_COMMAND_CREATE_RENDER_PROGRAM,
} resource_command_type;

typedef enum node_render_target {
  NODE_RENDER_TARGET_NONE = 1,
  NODE_RENDER_TARGET_PRESENT,
  NODE_RENDER_TARGET_IMAGE,
} node_render_target;

typedef enum mvk_image_sampler_usage {
  MVK_IMAGE_USAGE_NULL = 0,
  MVK_IMAGE_USAGE_READ_ONLY,
  MVK_IMAGE_USAGE_RENDER_TARGET_2D,
  MVK_IMAGE_USAGE_RENDER_TARGET_3D,
} mvk_image_sampler_usage;

// Shapes
typedef struct mc_point {
  int x, y;
} mc_point;

typedef struct mc_size {
  unsigned int width, height;
} mc_size;

typedef struct mc_rectf {
  float x, y;
  float width, height;
} mc_rect;

// Resources
struct font_resource;
struct mcr_texture_image;
struct mcr_render_program;
struct mcr_render_program_data;
typedef struct element_render_command {
  element_render_command_type type;
  unsigned int x, y;
  union {
    struct {
      unsigned int width, height;
      render_color color;
    } colored_rect_info;
    struct {
      char *text;
      font_resource *font;
      render_color color;
    } print_text;
    struct {
      unsigned int width, height;
      mcr_texture_image *texture;
    } textured_rect_info;
    // struct {
    //   float *world_matrix;
    //   mcr_vertex_buffer *vertex_buffer;

    //   unsigned int vertex_buffer;
    //   unsigned int index_buffer;
    //   unsigned int texture_uid;
    // } indexed_mesh;
    struct {
      mcr_render_program *program;
      mcr_render_program_data *data;

      // float *world_matrix;
      // unsigned int program_uid;
      // unsigned int vertex_buffer;
      // unsigned int index_buffer;
      // unsigned int texture_uid;
    } render_program;
  };
} element_render_command;

typedef struct image_render_details {
  unsigned int image_width, image_height;
  node_render_target render_target;
  render_color clear_color;
  unsigned int command_count;
  unsigned int commands_allocated;
  element_render_command *commands;
  union {
    struct {
      mcr_texture_image *image;

      // For render target images, given render commands are given in absolute screen coordinates
      // An offset is needed to apply to align it with the render target
      struct {
        unsigned int x, y;
      } screen_offset_coordinates;
    } target_image;
  } data;
} image_render_details;

typedef struct mcr_layout_binding {
  VkDescriptorType type;
  VkShaderStageFlags stage_bit;
  // If applicable
  size_t size_in_bytes;
} mcr_layout_binding;

typedef struct mcr_input_binding {
  VkFormat format;
  size_t size_in_bytes;
} mcr_input_binding;

typedef struct mcr_render_program_create_info {

  // TODO -- string versions / not filepaths // mandatory one is set / other is null
  char *vertex_shader_filepath;
  char *fragment_shader_filepath;

  // TODO -- make these arrays (& not alloc/release) for performance/clarity?
  unsigned int buffer_binding_count;
  mcr_layout_binding buffer_bindings[16];

  unsigned int input_binding_count;
  mcr_input_binding input_bindings[16];
} mcr_render_program_create_info;

typedef struct resource_command {
  resource_command_type type;
  void *p_resource;
  union {
    struct {
      const char *path;
    } load_texture;
    struct {
      unsigned int width, height;
      mvk_image_sampler_usage image_usage;
    } create_texture;
    struct {
      const char *path;
      float height;
    } font;
    struct {
      float *p_data;
      unsigned int data_count;
      bool release_original_data_on_copy;
    } load_mesh;
    struct {
      unsigned int *p_data;
      unsigned int data_count;
      bool release_original_data_on_copy;
    } load_indices;
    struct {
      mcr_render_program_create_info create_info;
    } create_render_program;
  };
} resource_command;

typedef struct resource_queue {
  pthread_mutex_t mutex;
  unsigned int count;
  unsigned int allocated;
  resource_command *commands;
} resource_queue;

typedef struct image_render_list {
  pthread_mutex_t mutex;
  unsigned int alloc, count;
  image_render_details **items;

} image_render_list;

struct loaded_font_list; // From mc_vulkan.h - compiled later
typedef struct render_thread_info {
  mthread_info *thread_info;
  image_render_list *image_queue;
  image_render_list *render_request_object_pool;
  resource_queue *resource_queue;
  // ptr reference only no need to release
  loaded_font_list *loaded_fonts;
  bool render_thread_initialized;
  window_input_buffer input_buffer;
} render_thread_info;

// struct tinyobj_obj;
// typedef struct mcr_model {
//   tinyobj_obj *parsed_obj;

//   unsigned int mesh_resource_uid;
// } mcr_model;

extern "C" {

int mcr_obtain_image_render_request(render_thread_info *image_render_queue, image_render_details **p_request);
int mcr_submit_image_render_request(render_thread_info *render_thread, image_render_details *request);
int mcr_obtain_element_render_command(image_render_details *image_queue, element_render_command **p_command);
int obtain_resource_command(resource_queue *resource_queue, resource_command **p_command);

void mcr_create_texture_resource(resource_queue *resource_queue, unsigned int width, unsigned int height,
                                 mvk_image_sampler_usage image_usage, mcr_texture_image **p_texture);
void mcr_load_texture_resource(const char *path, mcr_texture_image **p_texture);
void mcr_obtain_font_resource(resource_queue *resource_queue, const char *font_path, float font_height,
                              font_resource **font);
void mcr_create_render_program(mcr_render_program_create_info *create_info, mcr_render_program **p_resource);

void mcr_determine_text_display_dimensions(font_resource *font, const char *text, float *text_width,
                                           float *text_height);

void mcr_issue_render_command_text(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                   const char *text, font_resource *font, render_color font_color);
void mcr_issue_render_command_colored_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                           unsigned int width, unsigned int height, render_color color);
void mcr_issue_render_command_textured_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                            unsigned int width, unsigned int height, mcr_texture_image *texture);
}

#endif // MC_RENDER_COMMON_H