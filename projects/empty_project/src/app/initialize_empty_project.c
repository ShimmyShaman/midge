/* main_init.c */

#include <stdlib.h>

#include <unistd.h>

#include "core/core_definitions.h"
#include "core/midge_app.h"
#include "render/render_common.h"

#include "../projects/empty_project/src/app/initialize_empty_project.h"

void _mpt_render_present(image_render_details *image_render_queue, mc_node *node)
{
  empty_project_data *data = (empty_project_data *)node->data;

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, COLOR_BURLY_WOOD);
}

int initialize_empty_project(mc_node *node)
{
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 900;
  node->layout->preferred_height = 600;

  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&_mpt_render_present;

  mca_set_node_requires_layout_update(node);

  // cube_template
  empty_project_data *data = (empty_project_data *)malloc(sizeof(empty_project_data));
  node->data = data;
  data->node = node;

  return 0;
}