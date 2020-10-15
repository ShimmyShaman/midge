#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"

// extern "C" {
// }

void init_function_debug(mc_node *app_root)
{
  instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/function_debug.h", NULL);
  instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/function_debug.c", NULL);

  // TODO make an anonymous data register system
  // function_debug_pool *pool = (function_debug_pool *)malloc(sizeof(function_debug_pool));
  global_data->ui_state->function_debug_pool = (function_debug_pool *)malloc(sizeof(function_debug_pool));
  // mca_register_global_data_pointer("FUNCTION_DEBUG_POOL", pool);
}

void mce_activate_function_debugging(function_info *func_info)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  function_debug_pool *pool = (function_debug_pool *)global_data->ui_state->function_debug_pool;

  // TODO
  // TODO
  // TODO
  // TODO

  switch (definition->type) {
  case SOURCE_DEFINITION_FUNCTION: {
    // Determine if an editor already exists with this definition;
    mce_function_editor *function_editor = NULL;
    for (int i = 0; i < source_editor_pool->function_editor.size; ++i) {
      if (source_editor_pool->function_editor.items[i]->function == definition->data.func_info) {
        function_editor = source_editor_pool->function_editor.items[i];
        printf("feditor already existed\n");
        break;
      }
    }

    if (!function_editor) {
      _mce_obtain_function_editor_instance(source_editor_pool, &function_editor);

      _mce_set_definition_to_function_editor(function_editor, definition->data.func_info);
    }

    function_editor->node->layout->visible = true;
    mca_set_node_requires_layout_update(function_editor->node);

  } break;
  default:
    printf("[9766] NotSupported definition->type:%i\n", definition->type);
    return;
  }
}