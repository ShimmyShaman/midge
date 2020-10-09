
#include "core/core_definitions.h"
#include "modules/app_modules.h"
#include "render/render_common.h"

void mce_init_source_editor_pool()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mce_source_editor_pool *source_editor_pool = (mce_source_editor_pool *)malloc(sizeof(mce_source_editor_pool));
  global_data->ui_state->source_editor_pool = source_editor_pool;

  source_editor_pool->max_instance_count = 15;

  source_editor_pool->function_editor.size = 0;

  source_editor_pool->source_token_lists.capacity = 0;
  source_editor_pool->source_token_lists.count = 0;
  source_editor_pool->source_tokens.capacity = 0;
  source_editor_pool->source_tokens.count = 0;
}

void _mce_obtain_function_editor_instance(mce_source_editor_pool *source_editor_pool,
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

    global_root_data *global_data;
    obtain_midge_global_root(&global_data);

    mce_init_function_editor(global_data->global_node, source_editor_pool, function_editor);
    source_editor_pool->function_editor.items[source_editor_pool->function_editor.size - 1] = *function_editor;

    return;
  }

  MCerror(9945, "NotYetImplemented, use an older code editor instance");
}

void mce_obtain_source_token_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token **token)
{
  if (!source_editor_pool->source_tokens.count) {
    *token = (mce_source_token *)malloc(sizeof(mce_source_token));
    init_c_str(&(*token)->str);
  }
  else {
    --source_editor_pool->source_tokens.count;
    *token = source_editor_pool->source_tokens.items[source_editor_pool->source_tokens.count];
  }
}

void mce_obtain_source_token_list_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token_list **list)
{
  if (!source_editor_pool->source_token_lists.count) {
    *list = (mce_source_token_list *)malloc(sizeof(mce_source_token_list));
    (*list)->capacity = 0U;
    (*list)->count = 0U;
    return;
  }

  --source_editor_pool->source_token_lists.count;
  *list = source_editor_pool->source_token_lists.items[source_editor_pool->source_token_lists.count];
}

void mce_return_source_token_lists_to_editor_pool(mce_source_editor_pool *source_editor_pool,
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
}

extern "C" {
int _mce_set_definition_to_function_editor(mce_function_editor *function_editor, function_info *function);
}

void mce_activate_source_editor_for_definition(source_definition *definition)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

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