/* init_global_root.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycc/libtccinterp.h"

#include "core/core_definitions.h"

static mc_global_data *__mc_global_root = NULL;

int init_mc_global_data(TCCInterpState *tis)
{
  // TODO -- return error if any allocation returns NULL
  mc_node *global = (mc_node *)calloc(sizeof(mc_node), 1);
  global->type = NODE_TYPE_GLOBAL_ROOT;

  global->name = strdup("global");
  global->parent = NULL;
  global->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  global->children->count = 0;
  global->children->alloc = 0;
  global->children->items = NULL;

  __mc_global_root = (mc_global_data *)malloc(sizeof(mc_global_data));
  global->data = __mc_global_root;
  __mc_global_root->global_node = global;

  __mc_global_root->interpreter = tis;
  __mc_global_root->exit_requested = false;

  // "  __mc_global_root->children = (mc_node_list *)malloc(sizeof(mc_node_list));"
  // "  __mc_global_root->children->alloc = 0;"
  // "  __mc_global_root->children->count = 0;"

  __mc_global_root->source_files.alloc = 100;
  __mc_global_root->source_files.count = 0;
  __mc_global_root->source_files.items =
      (mc_source_file_info **)calloc(sizeof(mc_source_file_info *), __mc_global_root->source_files.alloc);

  __mc_global_root->functions.alloc = 100;
  __mc_global_root->functions.count = 0;
  __mc_global_root->functions.items =
      (function_info **)calloc(sizeof(function_info *), __mc_global_root->functions.alloc);

  __mc_global_root->function_declarations.alloc = 100;
  __mc_global_root->function_declarations.count = 0;
  __mc_global_root->function_declarations.items =
      (function_info **)calloc(sizeof(function_info *), __mc_global_root->function_declarations.alloc);

  __mc_global_root->structs.alloc = 30;
  __mc_global_root->structs.count = 0;
  __mc_global_root->structs.items = (struct_info **)calloc(sizeof(struct_info *), __mc_global_root->structs.alloc);

  __mc_global_root->enumerations.alloc = 20;
  __mc_global_root->enumerations.count = 0;
  __mc_global_root->enumerations.items =
      (enumeration_info **)calloc(sizeof(enumeration_info *), __mc_global_root->enumerations.alloc);

  __mc_global_root->preprocess_defines.alloc = 20;
  __mc_global_root->preprocess_defines.count = 0;
  __mc_global_root->preprocess_defines.items =
      (preprocess_define_info **)calloc(sizeof(preprocess_define_info *), __mc_global_root->preprocess_defines.alloc);

  __mc_global_root->event_handlers.alloc = 0;
  __mc_global_root->event_handlers.count = 0;

  return 0;
}

int obtain_midge_global_root(mc_global_data **root_data)
{
  *root_data = __mc_global_root;
  return 0;
}