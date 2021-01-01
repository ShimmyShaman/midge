/* ui_functionality.c */

#include <stdio.h>
#include <stdlib.h>

#include "core/midge_app.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mcu_initialize_ui_state(mcu_ui_state **p_ui_state)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  mcu_ui_state *ui_state = (mcu_ui_state *)malloc(sizeof(mcu_ui_state));

  // cache_layered_hit_list
  ui_state->cache_layered_hit_list = (mc_node_list *)malloc(sizeof(mc_node_list));
  ui_state->cache_layered_hit_list->alloc = 16;
  ui_state->cache_layered_hit_list->count = 0;
  ui_state->cache_layered_hit_list->items =
      (mc_node **)malloc(sizeof(mc_node *) * ui_state->cache_layered_hit_list->alloc);
  // printf("@creation global_data->ui_state:%p\n", global_data->ui_state);

  ui_state->default_font_resource = NULL;
  ui_state->requires_update = true;

  // Resource loading

  // Font
  printf("setting default font...\n");
  mcr_obtain_font_resource(global_data->render_thread->resource_queue, "res/font/DroidSansMono.ttf", 18.f,
                           &ui_state->default_font_resource);

  // Set
  *p_ui_state = ui_state;
}

// void mcu_initialize_core_ui_components()
// {
//   // mca_init_button_context_menu_options();
// }

void _mcu_get_interactive_nodes_within_node_at_point(mc_node *node, int screen_x, int screen_y,
                                                     mc_node_list *layered_hit_list)
{
  int a;
  mc_rectf *b;

  if (!node->layout || !node->layout->visible) {
    // A non-visible node
    return;
  }
  b = &node->layout->__bounds;
  // printf("%s (%i %i) %i %i %i %i\n", node->name, screen_x, screen_y, (int)b->x, (int)b->y, (int)(b->x + b->width),
  //        (int)(b->y + b->height));
  if (screen_x < (int)b->x || screen_y < (int)b->y || screen_x >= (int)(b->x + b->width) ||
      screen_y >= (int)(b->y + b->height))
    return;

  if (node->children) {
    for (a = node->children->count - 1; a >= 0; --a) {
      _mcu_get_interactive_nodes_within_node_at_point(node->children->items[a], screen_x, screen_y, layered_hit_list);
    }
  }

  append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count, node);
}

// Returns a list of ui-type nodes at the given point of the screen. Nodes at the nearer Z are earlier in the list.
void mcu_get_interactive_nodes_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  // Use the cache list
  // printf("layered_hit_list:%p\n", layered_hit_list);
  // printf("global_data->ui_state:%p\n", global_data->ui_state);
  // printf("global_data->ui_state->cache_layered_hit_list:%p\n", global_data->ui_state->cache_layered_hit_list);
  *layered_hit_list = global_data->ui_state->cache_layered_hit_list;
  (*layered_hit_list)->count = 0;

  _mcu_get_interactive_nodes_within_node_at_point(global_data->global_node, screen_x, screen_y, *layered_hit_list);

  // Focused Line Exception
  // -- Descendant focused items are permitted to have visual states that exceed their bounds (eg. a popup-menu from a
  //     small button), so the descendant line needs to be checked and ensured

  //  if (global_data->->layout->augmentation.count)
  // {
  //   mc_node *acn;
  //   for (a = 0; a < node->layout->augmentation.count; ++a) {
  //     acn = node->layout->augmentation.nodes[a];
  //     b = &acn->layout->__bounds;

  //     if (!acn->layout->visible)
  //       continue;

  //     if (screen_x < (int)b->x || screen_y < (int)b->y || screen_x >= (int)(b->x + b->width) ||
  //         screen_y >= (int)(b->y + b->height))
  //       continue;

  //     // Add
  //     append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count,
  //     acn);
  //   }
  // }

  // printf("mcu_get_interactive_nodes_at_point(%i, %i) : list_count:%i\n", screen_x, screen_y,
  //        (*layered_hit_list)->count);
}

// void mcu_init_ui_element(mc_node *parent_node, ui_element_type element_type, mcu_ui_element **created_element)
// {
//   mc_global_data *global_data;
//   obtain_midge_global_root(&global_data);

//   // Node
//   mc_node *node;
//   mca_init_mc_node(parent_node, NODE_TYPE_UI, &node);
//   mca_init_node_layout(&node->layout);

//   // if (parent_node->type == NODE_TYPE_UI) {
//   //   mcu_ui_element *parent_element = (mcu_ui_element *)parent_node->data;
//   //   switch (parent_element->type) {
//   //   case UI_ELEMENT_PANEL: {
//   //     mcu_panel *panel = (mcu_panel *)parent_element->data;

//   //     append_to_collection((void ***)&panel->children->items, &panel->children->alloc, &panel->children->count,
//   //     node); node->parent = parent_node;

//   //   } break;
//   //   case UI_ELEMENT_CONTEXT_MENU: {
//   //     mcu_context_menu *menu = (mcu_context_menu *)parent_element->data;

//   //     append_to_collection((void ***)&menu->children->items, &menu->children->alloc, &menu->children->count,
//   node);
//   //     node->parent = parent_node;
//   //   } break;
//   //   default: {
//   //     MCerror(1805, "mcu_init_ui_element::Unsupported type : %i", parent_element->type);
//   //   }
//   //   }
//   // }
//   // else {
//   // }
//   // // pthread_mutex_lock(&global_data->uid_counter.mutex);
//   // // node->uid = global_data->uid_counter.uid_counter++;
//   // // pthread_mutex_unlock(&global_data->uid_counter.mutex);

//   // UI Element
//   mcu_ui_element *element = (mcu_ui_element *)malloc(sizeof(mcu_ui_element));
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

// void mcu_get_hierarchical_children_node_list(mc_node *hierarchy_node, mc_node_list **children_node_list)
// {
//   mcu_ui_element *element = (mcu_ui_element *)hierarchy_node->data;

//   *children_node_list = NULL;
//   switch (element->type) {
//   case UI_ELEMENT_PANEL: {
//     mcu_panel *item = (mcu_panel *)element->data;
//     *children_node_list = item->children;
//   } break;
//   case UI_ELEMENT_CONTEXT_MENU: {
//     mcu_context_menu *item = (mcu_context_menu *)element->data;
//     *children_node_list = item->children;
//   } break;
//   default:
//     MCerror(8286, "TODO Support %i", element->type);
//   }
// }