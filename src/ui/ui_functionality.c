
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mui_initialize_ui_state()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_ui_state *ui_state = (mui_ui_state *)malloc(sizeof(mui_ui_state));
  global_data->ui_state = ui_state;

  // cache_layered_hit_list
  ui_state->cache_layered_hit_list = (mc_node_list *)malloc(sizeof(mc_node_list));
  ui_state->cache_layered_hit_list->alloc = 16;
  ui_state->cache_layered_hit_list->count = 0;
  ui_state->cache_layered_hit_list->items =
      (mc_node **)malloc(sizeof(mc_node *) * ui_state->cache_layered_hit_list->alloc);

  ui_state->default_font_resource = 0;

  // Resource loading
  pthread_mutex_lock(&global_data->render_thread->resource_queue.mutex);
  resource_command *command;

  // Font
  // printf("oius-0\n %p", global_data->render_thread->resource_queue);
  obtain_resource_command(&global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &ui_state->default_font_resource;
  command->data.font.height = 18;
  command->data.font.path = "res/font/DroidSansMono.ttf";

  pthread_mutex_unlock(&global_data->render_thread->resource_queue.mutex);
}

void mui_initialize_core_ui_components()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->ui_state = (mui_ui_state *)malloc(sizeof(mui_ui_state));

  // SAMPLE TEXTBLOCK
  mui_text_block *text_block;
  mui_init_text_block(global_data->global_node, &text_block);

  set_c_str(text_block->text, "Hello You!");
  mui_set_element_update(text_block->element);
}

void mui_update_ui()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->requires_rerender = true;
}

void mui_set_element_update(mui_ui_element *element)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Set update required on all ancestors of the node
  element->requires_update = true;
  mc_node *node = element->visual_node->parent;
  while (node) {
    switch (node->type) {
    case NODE_TYPE_GLOBAL_ROOT: {
      global_root_data *global_data = (global_root_data *)node->data;

      global_data->ui_state->requires_update = true;

      node = NULL;
    } break;
    default:
      MCerror(54, "mui_set_element_update::>unsupported node type:%i", node->type);
    }
  }
}

void _mui_get_ui_elements_within_node_at_point(mc_node *node, int screen_x, int screen_y,
                                               mc_node_list *layered_hit_list)
{
  // Including the node itself **
  switch (node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *global_data = (global_root_data *)node->data;

    if (screen_x < 0 || screen_y < 0 || screen_x >= global_data->screen.width || screen_y >= global_data->screen.height)
      break;

    // Add any children before
    for (int a = 0; a < global_data->children->count; ++a) {
      _mui_get_ui_elements_within_node_at_point(global_data->children->items[a], screen_x, screen_y, layered_hit_list);
    }

    append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count, node);
  } break;
  default:
    MCerror(27, "_mui_get_ui_elements_within_node_at_point::>unsupported node type:%i", node->type);
  }
}

// Returns a list of ui-type nodes at the given point of the screen. Nodes at the nearer Z are earlier in the list.
void mui_get_ui_elements_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Use the cache list
  *layered_hit_list = global_data->ui_state->cache_layered_hit_list;
  (*layered_hit_list)->count = 0;

  _mui_get_ui_elements_within_node_at_point(global_data->global_node, screen_x, screen_y, *layered_hit_list);

  // printf("mui_get_ui_elements_at_point(%i, %i) : list_count:%i\n", screen_x, screen_y, (*layered_hit_list)->count);
}

void mui_handle_mouse_left_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled)
{
  switch (ui_node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    // global_root_data *global_data = (global_root_data *)node->data;

    printf("global_node-left_click\n");
  } break;
  default:
    MCerror(69, "_mui_get_ui_elements_within_node_at_point::>unsupported node type:%i", ui_node->type);
  }
}

void mui_handle_mouse_right_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled)
{
  switch (ui_node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *global_data = (global_root_data *)ui_node->data;

  } break;
  default:
    MCerror(83, "_mui_get_ui_elements_within_node_at_point::>unsupported node type:%i", ui_node->type);
  }
}