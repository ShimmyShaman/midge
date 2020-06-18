/* node_render.h */
#ifndef NODE_RENDER_H
#define NODE_RENDER_H

#include "m_threads.h"

#define MCcall(function)                      \
  {                                           \
    int mc_res = function;                    \
    if (mc_res) {                             \
      printf("--" #function ":%i\n", mc_res); \
      return mc_res;                          \
    }                                         \
  }

#define MCerror(error_code, error_message, ...)                          \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

typedef struct mc_struct_id_v1 {
  const char *identifier;
  unsigned int version;
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

    memcpy(new_collection, *collection, *collection_count * sizeof(void *));
    free(*collection);

    *collection = new_collection;
    *collection_alloc = realloc_amount;
  }

  (*collection)[*collection_count] = item;
  ++*collection_count;
  return 0;
}

typedef enum render_command_type {
  RENDER_COMMAND_NONE = 1,
  RENDER_COMMAND_COLORED_RECTANGLE,
  RENDER_COMMAND_TEXTURED_RECTANGLE,
} render_command_type;

typedef struct render_color {
  float r, g, b, a;
} render_color;

typedef struct render_command {
  render_command_type type;
  unsigned int x, y, extent_w, extent_h;
  void *data;
} render_command;

typedef enum node_render_target {
  NODE_RENDER_TARGET_NONE = 1,
  NODE_RENDER_TARGET_HOST_IMAGE,
  NODE_RENDER_TARGET_PRESENT,
} node_render_target;

typedef struct node_render_sequence {
  unsigned int extent_width, extent_height;
  node_render_target render_target;
  int render_command_count;
  int render_commands_allocated;
  render_command *render_commands;
  void *image;
} node_render_sequence;

typedef struct renderer_queue {
  bool in_use;
  int count;
  node_render_sequence **items;
} renderer_queue;

typedef struct render_thread_info {
  mthread_info *thread_info;
  renderer_queue *renderer_queue;
  bool render_thread_initialized;
} render_thread_info;

#endif // NODE_RENDER_H