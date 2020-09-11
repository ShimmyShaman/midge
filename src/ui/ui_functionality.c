

#include "ui/ui_definitions.h"

void mui_initialize_ui_state()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->ui_state = (mui_ui_state *)malloc(sizeof(mui_ui_state));

  // cache_layered_hit_list
  global_data->ui_state->cache_layered_hit_list = (node_list *)malloc(sizeof(node_list));
  global_data->ui_state->cache_layered_hit_list->alloc = 16;
  global_data->ui_state->cache_layered_hit_list->count = 0;
  global_data->ui_state->cache_layered_hit_list->items =
      (node **)malloc(sizeof(node *) * global_data->ui_state->cache_layered_hit_list->alloc);
}

void mui_update_ui() { printf("mui_update_ui()\n"); }

void _mui_get_ui_elements_within_node_at_point(node *node, int screen_x, int screen_y, node_list *layered_hit_list)
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

// Returns a list of ui-type nodes at the given point of the screen. Nodes at the nearer Z are earlier.
void mui_get_ui_elements_at_point(int screen_x, int screen_y, node_list **layered_hit_list)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Use the cache list
  *layered_hit_list = global_data->ui_state->cache_layered_hit_list;
  (*layered_hit_list)->count = 0;

  _mui_get_ui_elements_within_node_at_point(global_data->global_node, screen_x, screen_y, *layered_hit_list);

  printf("mui_get_ui_elements_at_point(%i, %i) : list_count:%i\n", screen_x, screen_y, (*layered_hit_list)->count);
}

void mui_handle_mouse_left_click(node *ui_node, int screen_x, int screen_y, bool *handled)
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

void mui_handle_mouse_right_click(node *ui_node, int screen_x, int screen_y, bool *handled)
{

  switch (ui_node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    // global_root_data *global_data = (global_root_data *)node->data;

    printf("global_node-right_click\n");
  } break;
  default:
    MCerror(83, "_mui_get_ui_elements_within_node_at_point::>unsupported node type:%i", ui_node->type);
  }
}