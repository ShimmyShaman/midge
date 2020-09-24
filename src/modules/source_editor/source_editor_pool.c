
#include "core/core_definitions.h"
#include "modules/app_modules.h"
#include "render/render_common.h"

void mca_init_source_editor_pool()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mcm_source_editor_pool *source_editor_pool = (mcm_source_editor_pool *)malloc(sizeof(mcm_source_editor_pool));
  global_data->ui_state->source_editor_pool = source_editor_pool;

  source_editor_pool->max_instance_count = 15;
  source_editor_pool->size = 0;
}

void _mcm_obtain_function_editor_instance(mcm_source_editor_pool *source_editor_pool,
                                          mcm_function_editor **function_editor)
{
  for (int i = 0; i < source_editor_pool->size; ++i) {
    if (!source_editor_pool->instances[i]->node->layout->visible) {
      *function_editor = source_editor_pool->instances[i];
    }
  }

  if (source_editor_pool->size < source_editor_pool->max_instance_count) {
    // Construct a new instance
    reallocate_collection((void ***)&source_editor_pool->instances, &source_editor_pool->size,
                          source_editor_pool->size + 1, 0);

    global_root_data *global_data;
    obtain_midge_global_root(&global_data);

    mcm_init_function_editor(global_data->global_node, source_editor_pool, function_editor);
    source_editor_pool->instances[source_editor_pool->size - 1] = *function_editor;

    return;
  }

  MCerror(9945, "NotYetImplemented, use an older code editor instance");
}

extern "C" {
int _mcm_set_definition_to_function_editor(mcm_function_editor *function_editor, function_info *function);
}

void mca_activate_source_editor_for_definition(source_definition *definition)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mcm_source_editor_pool *source_editor_pool = (mcm_source_editor_pool *)global_data->ui_state->source_editor_pool;

  switch (definition->type) {
  case SOURCE_DEFINITION_FUNCTION: {
    mcm_function_editor *function_editor;
    _mcm_obtain_function_editor_instance(source_editor_pool, &function_editor);

    function_editor->node->layout->visible = true;
    mca_set_node_requires_layout_update(function_editor->node);

    _mcm_set_definition_to_function_editor(function_editor, definition->data.func_info);
  } break;
  default:
    printf("[9766] NotSupported definition->type:%i\n", definition->type);
    return;
  }
}