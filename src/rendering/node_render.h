/* node_render.h */
#ifndef NODE_RENDER_H
#define NODE_RENDER_H

#include "midge_common.h"

#define MCcall(function)                      \
  {                                           \
    int mc_res = function;                    \
    if (mc_res) {                             \
      printf("--" #function ":%i\n", mc_res); \
      return mc_res;                          \
    }                                         \
  }

#define MCvacall(function)                                            \
  {                                                                   \
    int mc_res = function;                                            \
    if (mc_res) {                                                     \
      printf("-- line:%d varg-function-call:%i\n", __LINE__, mc_res); \
      return mc_res;                                                  \
    }                                                                 \
  }

#define MCerror(error_code, error_message, ...)                          \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

typedef struct mc_struct_id_v1 {
  const char *identifier;
  unsigned short version;
} mc_struct_id_v1;

typedef struct mc_void_collection_v1 {
  mc_struct_id_v1 *struct_id;
  unsigned int allocated;
  unsigned int count;
  void **items;
} mc_void_collection_v1;

int append_to_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count, void *item)
{
  if (*collection_count + 1 > *collection_alloc) {
    unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
    // printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
    if (new_collection == NULL) {
      MCerror(304, "append_to_collection malloc error");
    }

    if (*collection_alloc) {
      memcpy(new_collection, *collection, *collection_count * sizeof(void *));
      free(*collection);
    }

    *collection = new_collection;
    *collection_alloc = realloc_amount;
  }

  (*collection)[*collection_count] = item;
  ++*collection_count;
  return 0;
}

int insert_in_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
                         int insertion_index, void *item)
{
  if (*collection_count + 1 > *collection_alloc) {
    unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
    // printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
    if (new_collection == NULL) {
      MCerror(304, "append_to_collection malloc error");
    }

    if (*collection_alloc) {
      memcpy(new_collection, *collection, *collection_count * sizeof(void *));
      free(*collection);
    }

    *collection = new_collection;
    *collection_alloc = realloc_amount;
  }

  for (int i = *collection_count; i > insertion_index; --i) {
    (*collection)[i] = (*collection)[i - 1];
  }
  (*collection)[insertion_index] = item;
  ++*collection_count;
  return 0;
}

int remove_from_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
                           int index)
{
  *collection[index] = NULL;
  for (int i = index + 1; i < *collection_count; ++i)
    *collection[i - 1] = *collection[i];

  --*collection_count;
  if (index > 0)
    *collection[*collection_count] = NULL;

  return 0;
}

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

#endif // NODE_RENDER_H