/* render_thread.h */

#ifndef MC_RENDER_THREAD_H
#define MC_RENDER_THREAD_H

#include "m_threads.h"
#include "midge_common.h"
#include "platform/mc_xcb.h"
#include "render/render_common.h"

#define MRT_SEQUENCE_COPY_BUFFER_SIZE 8192

typedef enum element_render_command_type {
  RENDER_COMMAND_NONE = 1,
  RENDER_COMMAND_COLORED_QUAD,
  RENDER_COMMAND_TEXTURED_QUAD,
  RENDER_COMMAND_PRINT_TEXT,
} element_render_command_type;

typedef enum resource_command_type {
  RESOURCE_COMMAND_NONE = 1,
  RESOURCE_COMMAND_LOAD_TEXTURE,
  RESOURCE_COMMAND_CREATE_TEXTURE,
  RESOURCE_COMMAND_LOAD_FONT,
} resource_command_type;

typedef enum node_render_target {
  NODE_RENDER_TARGET_NONE = 1,
  NODE_RENDER_TARGET_PRESENT,
  NODE_RENDER_TARGET_IMAGE,
} node_render_target;

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
      unsigned int font_resource_uid;
      render_color color;
    } print_text;
    struct {
      unsigned int width, height;
      unsigned int texture_uid;
    } textured_rect_info;
  } data;
} element_render_command;

typedef struct image_render_queue {
  unsigned int image_width, image_height;
  node_render_target render_target;
  render_color clear_color;
  unsigned int command_count;
  unsigned int commands_allocated;
  element_render_command *commands;
  union {
    struct {
      unsigned int image_uid;
    } target_image;
  } data;
} image_render_queue;

typedef struct resource_command {
  resource_command_type type;
  unsigned int *p_uid;
  union {
    struct {
      const char *path;
    } load_texture;
    struct {
      unsigned int width, height;
      bool use_as_render_target;
    } create_texture;
    struct {
      const char *path;
      float height;
    } font;
  } data;
} resource_command;

typedef struct resource_queue {
  pthread_mutex_t mutex;
  unsigned int count;
  unsigned int allocated;
  resource_command *commands;
} resource_queue;

typedef struct render_queue {
  pthread_mutex_t mutex;
  unsigned int count;
  unsigned int allocated;
  image_render_queue *image_renders;
} render_queue;

typedef struct render_thread_info {
  mthread_info *thread_info;
  render_queue render_queue;
  resource_queue resource_queue;
  bool render_thread_initialized;
  window_input_buffer input_buffer;
} render_thread_info;

typedef struct mrt_sequence_copy_buffer {
  VkDescriptorBufferInfo vpc_desc_buffer_info;
  u_char data[MRT_SEQUENCE_COPY_BUFFER_SIZE];
  u_int32_t index;
} mrt_sequence_copy_buffer;

typedef struct vert_data_scale_offset {
  struct {
    float x, y;
  } offset;
  struct {
    float x, y;
  } scale;
} vert_data_scale_offset;

typedef struct frag_ubo_tint_texcoordbounds {
  struct {
    float r, g, b, a;
  } tint;
  struct {
    float s0, s1, t0, t1;
  } tex_coord_bounds;
} frag_ubo_tint_texcoordbounds;

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

extern "C" {
void *midge_render_thread(void *vargp);

int obtain_resource_command(resource_queue *resource_queue, resource_command **p_command);
int obtain_image_render_queue(render_queue *render_queue, image_render_queue **p_command);
int obtain_element_render_command(image_render_queue *image_queue, element_render_command **p_command);
}

#endif // RENDER_THREAD_H