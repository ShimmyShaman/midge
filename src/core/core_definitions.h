/* core_definitions.h */

#ifndef CORE_DEFINITIONS_H
#define CORE_DEFINITIONS_H

#include "core/midge_core.h"

typedef struct struct_id {
  const char *identifier;
  unsigned short version;
} struct_id;

typedef struct node {
  struct_id *struct_id;
  const char *name;

  node *parent;
  unsigned int children_alloc;
  unsigned int child_count;
  node **children;

  void *data;
};

//   unsigned int functions_alloc;
//   unsigned int function_count;
//   mc_function_info_v1 **functions;
//   unsigned int structs_alloc;
//   unsigned int struct_count;
//   mc_struct_info_v1 **structs;

//   node_type type;
//   union {
//     node_visual_info visual;
//     struct {
//       uint image_resource_uid;
//       int (*render_delegate)(int, void **);
//     } global_root;
//   } data;

//   struct {
//     uint alloc, count;
//     event_handler_array **items;
//   } event_handlers;

//   void *extra;
}
mc_node_v1;

#endif // CORE_DEFINITIONS_H