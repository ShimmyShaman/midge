/* node_render.h */
#ifndef NODE_RENDER_H
#define NODE_RENDER_H

#include "m_threads.h"

typedef struct node_render_sequence {
  unsigned int extent_width, extent_height;
  int render_commands_allocated;
  render_command *render_commands;
  void *image;
} node_render_sequence;

typedef enum render_command_type {
  Null = 0,
  NONE,
  COLORED_RECTANGLE,
  TEXTURED_RECTANGLE,
} render_command_type;

typedef struct render_color {
  float r, g, b, a;
} render_color;

typedef struct render_command {
  render_command_type type;
  unsigned int x, y, extent_w, extent_h;
  void *data;
} render_command;

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