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

int _mcu_get_interactive_nodes_within_node_at_point(mc_node *node, int screen_x, int screen_y,
                                                    mc_node_list *layered_hit_list)
{
  int a;
  mc_rectf *b;

  if (!node->layout || !node->layout->visible) {
    // A non-visible node
    return 0;
  }
  b = &node->layout->__bounds;
  // printf("%s (%i %i) %i %i %i %i\n", node->name, screen_x, screen_y, (int)b->x, (int)b->y, (int)(b->x + b->width),
  //        (int)(b->y + b->height));
  if (screen_x < (int)b->x || screen_y < (int)b->y || screen_x >= (int)(b->x + b->width) ||
      screen_y >= (int)(b->y + b->height))
    return 0;

  if (node->children) {
    for (a = node->children->count - 1; a >= 0; --a) {
      MCcall(_mcu_get_interactive_nodes_within_node_at_point(node->children->items[a], screen_x, screen_y,
                                                             layered_hit_list));
    }
  }

  MCcall(append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count,
                              node));
  return 0;
}

// Returns a list of ui-type nodes at the given point of the screen. Nodes at the nearer Z are earlier in the list.
int mcu_get_interactive_nodes_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  // Use the cache list
  // printf("layered_hit_list:%p\n", layered_hit_list);
  // printf("global_data->ui_state:%p\n", global_data->ui_state);
  // printf("global_data->ui_state->cache_layered_hit_list:%p\n", global_data->ui_state->cache_layered_hit_list);
  *layered_hit_list = global_data->ui_state->cache_layered_hit_list;
  (*layered_hit_list)->count = 0;

  MCcall(
      _mcu_get_interactive_nodes_within_node_at_point(global_data->global_node, screen_x, screen_y, *layered_hit_list));

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

  return 0;
}