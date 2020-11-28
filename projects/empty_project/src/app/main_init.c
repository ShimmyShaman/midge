/* main_init.c */

#include "main_init.h"

int initialize_app()
{
  mc_node *node;
  mca_init_mc_node(NODE_TYPE_ABSTRACT, "empty_root", &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 900;
  node->layout->preferred_height = 600;

  node->layout->padding.left = 400;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_cbt_render_td_ct_data_headless;
  node->layout->render_present = (void *)&_cbt_render_td_ct_data_present;

  mca_set_node_requires_layout_update(node);

  // cube_template
  cube_template_root_data *ct_data = (cube_template_root_data *)malloc(sizeof(cube_template_root_data));
  node->data = ct_data;
  ct_data->node = node;

  ct_data->render_target.image = NULL;
  ct_data->render_target.width = node->layout->preferred_width;
  ct_data->render_target.height = node->layout->preferred_height;
  mcr_create_texture_resource(ct_data->render_target.width, ct_data->render_target.height,
                              MVK_IMAGE_USAGE_RENDER_TARGET_3D, &ct_data->render_target.image);
}