/* main_init.c */

#include <stdlib.h>

#include "core/core_definitions.h"
#include "core/midge_app.h"
#include "render/render_common.h"

#include "../projects/empty_project/src/app/initialize_empty_project.h"

void _cbt_render_td_ct_data_headless(mc_node *node)
{
  cube_template_root_data *td_ct_data = (cube_template_root_data *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }

  // Render the render target
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  image_render_details *irq;
  mcr_obtain_image_render_request(global_data->render_thread, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_CORNFLOWER_BLUE;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = td_ct_data->render_target.width;   // TODO
  irq->image_height = td_ct_data->render_target.height; // TODO
  irq->data.target_image.image = td_ct_data->render_target.image;
  irq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  irq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_present(irq, child);
    }
  }

  // mcr_render_model(irq, td_ct_data->cube.model);

  mcr_submit_image_render_request(global_data->render_thread, irq);
}

void _cbt_render_td_ct_data_present(image_render_details *image_render_queue, mc_node *node)
{
  cube_template_root_data *td_ct_data = (cube_template_root_data *)node->data;

  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, td_ct_data->render_target.width,
                                         td_ct_data->render_target.height, td_ct_data->render_target.image);

  // mca_set_node_requires_rerender(node);
}

int initialize_empty_project()
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

  return 0;
}