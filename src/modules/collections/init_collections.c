/* init_collections.c */

#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

// #include "modules/function_debug/function_debug.h"

// extern "C" {
// }

int init_collections(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/collections/hash_table.h"));
  MCcall(mcs_interpret_file("src/modules/collections/hash_table.c"));

  // TODO make an anonymous data register system
  // function_debug_pool *pool = (function_debug_pool *)malloc(sizeof(function_debug_pool));
  // global_data->ui_state->function_debug_pool = (mce_function_debug_pool *)malloc(sizeof(mce_function_debug_pool));
  // mca_register_global_data_pointer("FUNCTION_DEBUG_POOL", pool);
  return 0;
}