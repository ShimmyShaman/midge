/* init_global_root.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinycc/libtccinterp.h"

#include "core/core_definitions.h"

static mc_app_itp_data *__mc_app_itp_data = NULL;

int mc_init_app_itp_data(TCCInterpState *tis)
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

  __mc_app_itp_data = (mc_app_itp_data *)malloc(sizeof(mc_app_itp_data));
  global->data = __mc_app_itp_data;
  __mc_app_itp_data->global_node = global;

  __mc_app_itp_data->interpreter = tis;

  // "  __mc_app_itp_data->children = (mc_node_list *)malloc(sizeof(mc_node_list));"
  // "  __mc_app_itp_data->children->alloc = 0;"
  // "  __mc_app_itp_data->children->count = 0;"

  __mc_app_itp_data->source_files.alloc = 100;
  __mc_app_itp_data->source_files.count = 0;
  __mc_app_itp_data->source_files.items =
      (mc_source_file_info **)calloc(sizeof(mc_source_file_info *), __mc_app_itp_data->source_files.alloc);

  __mc_app_itp_data->functions.alloc = 100;
  __mc_app_itp_data->functions.count = 0;
  __mc_app_itp_data->functions.items =
      (function_info **)calloc(sizeof(function_info *), __mc_app_itp_data->functions.alloc);

  __mc_app_itp_data->function_declarations.alloc = 100;
  __mc_app_itp_data->function_declarations.count = 0;
  __mc_app_itp_data->function_declarations.items =
      (function_info **)calloc(sizeof(function_info *), __mc_app_itp_data->function_declarations.alloc);

  __mc_app_itp_data->structs.alloc = 30;
  __mc_app_itp_data->structs.count = 0;
  __mc_app_itp_data->structs.items = (struct_info **)calloc(sizeof(struct_info *), __mc_app_itp_data->structs.alloc);

  __mc_app_itp_data->enumerations.alloc = 20;
  __mc_app_itp_data->enumerations.count = 0;
  __mc_app_itp_data->enumerations.items =
      (enumeration_info **)calloc(sizeof(enumeration_info *), __mc_app_itp_data->enumerations.alloc);

  __mc_app_itp_data->preprocess_defines.alloc = 20;
  __mc_app_itp_data->preprocess_defines.count = 0;
  __mc_app_itp_data->preprocess_defines.items =
      (preprocess_define_info **)calloc(sizeof(preprocess_define_info *), __mc_app_itp_data->preprocess_defines.alloc);

  __mc_app_itp_data->event_handlers.alloc = 0;
  __mc_app_itp_data->event_handlers.count = 0;

  return 0;
}

int mc_obtain_app_itp_data(mc_app_itp_data **p_data)
{
  *p_data = __mc_app_itp_data;
  return 0;
}