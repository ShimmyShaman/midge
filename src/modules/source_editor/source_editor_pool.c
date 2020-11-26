/* source_editor_pool.c */

#include <stdio.h>
#include <stdlib.h>

#include "core/app_modules.h"
#include "core/core_definitions.h"
#include "core/midge_app.h"

#include "render/render_common.h"

#include "modules/source_editor/source_editor.h"

// Forward Declarations
int _mce_set_definition_to_function_editor(mce_function_editor *function_editor, function_info *function);

int mce_init_source_editor_pool()
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  mce_source_editor_pool *source_editor_pool = (mce_source_editor_pool *)malloc(sizeof(mce_source_editor_pool));
  global_data->ui_state->source_editor_pool = source_editor_pool;

  source_editor_pool->max_instance_count = 15;

  source_editor_pool->function_editor.size = 0;

  source_editor_pool->source_token_lists.capacity = 0;
  source_editor_pool->source_token_lists.count = 0;
  source_editor_pool->source_tokens.capacity = 0;
  source_editor_pool->source_tokens.count = 0;

  return 0;
}

int _mce_obtain_function_editor_instance(mce_source_editor_pool *source_editor_pool,
                                         mce_function_editor **function_editor)
{
  for (int i = 0; i < source_editor_pool->function_editor.size; ++i) {
    if (!source_editor_pool->function_editor.items[i]->node->layout->visible) {
      *function_editor = source_editor_pool->function_editor.items[i];
    }
  }

  if (source_editor_pool->function_editor.size < source_editor_pool->max_instance_count) {
    // Construct a new instance
    reallocate_collection((void ***)&source_editor_pool->function_editor.items,
                          &source_editor_pool->function_editor.size, source_editor_pool->function_editor.size + 1, 0);

    midge_app_info *global_data;
    mc_obtain_midge_app_info(&global_data);

    mce_init_function_editor(global_data->global_node, source_editor_pool, function_editor);
    source_editor_pool->function_editor.items[source_editor_pool->function_editor.size - 1] = *function_editor;

    return 0;
  }

  MCerror(9945, "NotYetImplemented, use an older code editor instance");
}

int mce_obtain_source_token_list_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token_list **list)
{
  if (!source_editor_pool->source_token_lists.count) {
    *list = (mce_source_token_list *)malloc(sizeof(mce_source_token_list));
    (*list)->capacity = 0U;
    (*list)->count = 0U;
    return 0;
  }

  --source_editor_pool->source_token_lists.count;
  *list = source_editor_pool->source_token_lists.items[source_editor_pool->source_token_lists.count];

  return 0;
}

int mce_return_source_token_lists_to_editor_pool(mce_source_editor_pool *source_editor_pool,
                                                 mce_source_token_list **lists, unsigned int count)
{
  for (int a = 0; a < count; ++a) {
    mce_source_token_list *list = lists[a];

    for (int b = 0; b < list->count; ++b) {
      // Return the source tokens to the pool
      append_to_collection((void ***)&source_editor_pool->source_tokens.items,
                           &source_editor_pool->source_tokens.capacity, &source_editor_pool->source_tokens.count,
                           list->items[b]);
    }

    // Return the list to the pool
    append_to_collection((void ***)&source_editor_pool->source_token_lists.items,
                         &source_editor_pool->source_token_lists.capacity,
                         &source_editor_pool->source_token_lists.count, list);
  }

  return 0;
}

int mce_activate_source_editor_for_definition(source_definition *definition)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  mce_source_editor_pool *source_editor_pool = (mce_source_editor_pool *)global_data->ui_state->source_editor_pool;

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
      MCcall(_mce_obtain_function_editor_instance(source_editor_pool, &function_editor));

      _mce_set_definition_to_function_editor(function_editor, definition->data.func_info);
    }

    function_editor->node->layout->visible = true;
    mca_set_node_requires_layout_update(function_editor->node);

  } break;
  default:
    MCerror(9766, "NotSupported definition->type:%i\n", definition->type);
  }

  return 0;
}