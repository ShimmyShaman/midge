#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"

// extern "C" {
// }

void init_function_debug(mc_node *app_root)
{
  instantiate_all_definitions_from_file(app_root, "src/modules/function_debug/function_debug.h", NULL);
  instantiate_all_definitions_from_file(app_root, "src/modules/function_debug/function_debug.c", NULL);

  // TODO make an anonymous data register system
  // function_debug_pool *pool = (function_debug_pool *)malloc(sizeof(function_debug_pool));
  // global_data->ui_state->function_debug_pool = (mce_function_debug_pool *)malloc(sizeof(mce_function_debug_pool));
  // mca_register_global_data_pointer("FUNCTION_DEBUG_POOL", pool);
}