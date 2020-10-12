
#include "env/environment_definitions.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mui_initialize_ui_state(mui_ui_state **p_ui_state)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_ui_state *ui_state = (mui_ui_state *)malloc(sizeof(mui_ui_state));

  // cache_layered_hit_list
  ui_state->cache_layered_hit_list = (mc_node_list *)malloc(sizeof(mc_node_list));
  ui_state->cache_layered_hit_list->alloc = 16;
  ui_state->cache_layered_hit_list->count = 0;
  ui_state->cache_layered_hit_list->items =
      (mc_node **)malloc(sizeof(mc_node *) * ui_state->cache_layered_hit_list->alloc);
  // printf("@creation global_data->ui_state:%p\n", global_data->ui_state);

  ui_state->default_font_resource = 0;
  ui_state->requires_update = true;

  // Resource loading

  // Font
  printf("setting default font...\n");
  mcr_obtain_font_resource(global_data->render_thread->resource_queue, "res/font/DroidSansMono.ttf", 18.f,
                           &ui_state->default_font_resource);

  // Set
  *p_ui_state = ui_state;
}

void mui_initialize_core_ui_components() { mca_init_button_context_menu_options(); }

void _mui_get_interactive_nodes_within_node_at_point(mc_node *node, int screen_x, int screen_y,
                                                     mc_node_list *layered_hit_list)
{
  if (!node->layout) {
    // A non-visible node
    return;
  }

  if (screen_x < (int)node->layout->__bounds.x || screen_y < (int)node->layout->__bounds.y ||
      screen_x >= (int)(node->layout->__bounds.x + node->layout->__bounds.width) ||
      screen_y >= (int)(node->layout->__bounds.y + node->layout->__bounds.height))
    return;

  if (node->children) {
    for (int a = node->children->count - 1; a >= 0; --a) {
      _mui_get_interactive_nodes_within_node_at_point(node->children->items[a], screen_x, screen_y, layered_hit_list);
    }
  }

  append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count, node);
}

// Returns a list of ui-type nodes at the given point of the screen. Nodes at the nearer Z are earlier in the list.
void mui_get_interactive_nodes_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Use the cache list
  // printf("layered_hit_list:%p\n", layered_hit_list);
  // printf("global_data->ui_state:%p\n", global_data->ui_state);
  // printf("global_data->ui_state->cache_layered_hit_list:%p\n", global_data->ui_state->cache_layered_hit_list);
  *layered_hit_list = global_data->ui_state->cache_layered_hit_list;
  (*layered_hit_list)->count = 0;

  _mui_get_interactive_nodes_within_node_at_point(global_data->global_node, screen_x, screen_y, *layered_hit_list);

  // printf("mui_get_interactive_nodes_at_point(%i, %i) : list_count:%i\n", screen_x, screen_y,
  //        (*layered_hit_list)->count);
}

// void mui_init_ui_element(mc_node *parent_node, ui_element_type element_type, mui_ui_element **created_element)
// {
//   global_root_data *global_data;
//   obtain_midge_global_root(&global_data);

//   // Node
//   mc_node *node;
//   mca_init_mc_node(parent_node, NODE_TYPE_UI, &node);
//   mca_init_node_layout(&node->layout);

//   // if (parent_node->type == NODE_TYPE_UI) {
//   //   mui_ui_element *parent_element = (mui_ui_element *)parent_node->data;
//   //   switch (parent_element->type) {
//   //   case UI_ELEMENT_PANEL: {
//   //     mui_panel *panel = (mui_panel *)parent_element->data;

//   //     append_to_collection((void ***)&panel->children->items, &panel->children->alloc, &panel->children->count,
//   //     node); node->parent = parent_node;

//   //   } break;
//   //   case UI_ELEMENT_CONTEXT_MENU: {
//   //     mui_context_menu *menu = (mui_context_menu *)parent_element->data;

//   //     append_to_collection((void ***)&menu->children->items, &menu->children->alloc, &menu->children->count,
//   node);
//   //     node->parent = parent_node;
//   //   } break;
//   //   default: {
//   //     MCerror(1805, "mui_init_ui_element::Unsupported type : %i", parent_element->type);
//   //   }
//   //   }
//   // }
//   // else {
//   // }
//   // // pthread_mutex_lock(&global_data->uid_counter.mutex);
//   // // node->uid = global_data->uid_counter.uid_index++;
//   // // pthread_mutex_unlock(&global_data->uid_counter.mutex);

//   // UI Element
//   mui_ui_element *element = (mui_ui_element *)malloc(sizeof(mui_ui_element));
//   node->data = element;

//   {
//     // Initialize layout
//     element->layout = (mca_node_layout *)malloc(sizeof(mca_node_layout));

//     element->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
//     element->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
//     element->layout->preferred_width = 0;
//     element->layout->preferred_height = 0;
//     // element->layout->min_width = 0;
//     // element->layout->min_height = 0;
//     // element->layout->max_width = 0;
//     // element->layout->max_height = 0;
//     element->layout->padding = {0, 0, 0, 0};
//   }
//   element->visual_node = node;
//   element->type = element_type;
//   element->requires_rerender = false;

//   element->data = NULL;

//   mca_set_node_requires_layout_update(node);

//   if (created_element)
//     *created_element = element;
// }

// void mui_get_hierarchical_children_node_list(mc_node *hierarchy_node, mc_node_list **children_node_list)
// {
//   mui_ui_element *element = (mui_ui_element *)hierarchy_node->data;

//   *children_node_list = NULL;
//   switch (element->type) {
//   case UI_ELEMENT_PANEL: {
//     mui_panel *item = (mui_panel *)element->data;
//     *children_node_list = item->children;
//   } break;
//   case UI_ELEMENT_CONTEXT_MENU: {
//     mui_context_menu *item = (mui_context_menu *)element->data;
//     *children_node_list = item->children;
//   } break;
//   default:
//     MCerror(8286, "TODO Support %i", element->type);
//   }
// }