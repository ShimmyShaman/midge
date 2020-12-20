/* init_render_utilities.c */

#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"



int init_render_utilities(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/render_utilities/render_util.h"));
  MCcall(mcs_interpret_file("src/modules/render_utilities/render_util.c"));

  // TODO make an anonymous data register system
  // function_debug_pool *pool = (function_debug_pool *)malloc(sizeof(function_debug_pool));
  // global_data->ui_state->function_debug_pool = (mce_function_debug_pool *)malloc(sizeof(mce_function_debug_pool));
  // mca_register_global_data_pointer("FUNCTION_DEBUG_POOL", pool);
  return 0;
}