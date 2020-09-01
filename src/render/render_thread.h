
#include "m_threads.h"
#include "midge_common.h"
#include "platform/mc_xcb.h"

typedef enum element_render_command_type {
  RENDER_COMMAND_NONE = 1,
  RENDER_COMMAND_BACKGROUND_COLOR,
  RENDER_COMMAND_SAMPLE_CUBE,
  RENDER_COMMAND_COLORED_RECTANGLE,
  RENDER_COMMAND_TEXTURED_RECTANGLE,
  RENDER_COMMAND_PRINT_TEXT,
} element_render_command_type;

typedef struct render_color {
  float r, g, b, a;
} render_color;

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
      uint font_resource_uid;
      render_color color;
    } print_text;
    struct {
      unsigned int width, height;
      uint texture_uid;
    } textured_rect_info;
  } data;
} element_render_command;

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

typedef struct image_render_queue {
  unsigned int image_width, image_height;
  node_render_target render_target;
  render_color clear_color;
  uint command_count;
  uint commands_allocated;
  element_render_command *commands;
  union {
    struct {
      uint image_uid;
    } target_image;
  } data;
} image_render_queue;

typedef struct resource_command {
  resource_command_type type;
  uint *p_uid;
  union {
    struct {
      const char *path;
    } load_texture;
    struct {
      uint width, height;
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

extern "C" {
void *midge_render_thread(void *vargp);
}