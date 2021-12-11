/* panel.c */

#include <stdio.h>
#include <stdlib.h>

#include "control/mc_controller.h"
#include "env/environment_definitions.h"

#include "modules/ui_elements/panel.h"

void _mcu_render_panel_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_panel *panel = (mcu_panel *)node->data;

  // printf("panel:-x:%f y:%f width:%f height:%f\n", node->layout->__bounds.x, node->layout->__bounds.y,
  //        node->layout->__bounds.width, node->layout->__bounds.height);

  // Background
  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, panel->background_color);

  if (node->children && node->children->count) {
    mca_render_node_list_present(image_render_queue, node->children);
  }
}

static void _mcu_panel_destroy_data(void *data)
{
  // mcu_panel *panel = (mcu_panel *)data;

  free(data);
}

int mcu_init_panel(mc_node *parent, mcu_panel **p_panel)
{
  // Node
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_MCU_PANEL, "unnamed-panel", &node));
  node->destroy_data = (void *)&_mcu_panel_destroy_data;
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  // Layout
  MCcall(mca_init_node_layout(&node->layout));
  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&_mcu_render_panel_present;
  node->layout->handle_input_event = NULL; //(void *)&_mcu_panel_handle_input_event;

  // Default Settings
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  // Control
  mcu_panel *panel = (mcu_panel *)malloc(sizeof(mcu_panel)); // TODO -- malloc check
  panel->node = node;
  node->data = panel;

  // printf("mcu-ib-3\n");
  panel->tag = NULL;

  panel->background_color = COLOR_LIGHT_YELLOW;

  // Set to out pointer
  *p_panel = panel;

  if (parent) {
    MCcall(mca_attach_node_to_hierarchy(parent, node));
  }

  return 0;
}