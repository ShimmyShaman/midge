
/* render_thread.h */

#ifndef MC_RENDER_COMMON_H
#define MC_RENDER_COMMON_H

#include "midge_common.h"
#include "platform/mc_xcb.h"

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

typedef struct mc_rectf {
  float x, y;
  float width, height;
} mc_rect;

typedef struct mc_size {
  unsigned int width, height;
} mc_size;

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

extern "C" {

int obtain_resource_command(resource_queue *resource_queue, resource_command **p_command);
int obtain_image_render_queue(render_queue *render_queue, image_render_queue **p_command);
int obtain_element_render_command(image_render_queue *image_queue, element_render_command **p_command);

void mcr_create_texture_resource(resource_queue *resource_queue, unsigned int width, unsigned int height,
                                 bool use_as_render_target, unsigned int *resource_uid);
}

#endif // MC_RENDER_COMMON_H