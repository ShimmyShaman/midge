#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"

typedef struct mc_td_portal {
  mc_node *node;

  struct {
    unsigned int width, height;
    unsigned int resource_uid;
  } render_target;
} mc_td_portal;

void __mc_td_determine_td_portal_extents(mc_node *node, layout_extent_restraints restraints)
{
  mc_td_portal *td_portal = (mc_td_portal *)node->data;

  // Determine
  if (node->layout->preferred_width) {
    node->layout->determined_extents.width = node->layout->preferred_width;
  }
  else {
    MCerror(7525, "NotYetSupported");
  }
  if (node->layout->preferred_height) {
    node->layout->determined_extents.height = node->layout->preferred_height;
  }
  else {
    MCerror(8932, "NotYetSupported");
  }
}

void __mc_td_update_td_portal_layout(mc_node *node, mc_rectf *available_area)
{
  mc_td_portal *td_portal = (mc_td_portal *)node->data;

  mca_update_typical_node_layout(node, available_area);
}

void __mc_td_render_td_portal_headless(mc_node *node)
{
  mc_td_portal *td_portal = (mc_td_portal *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }

  // Render the render target
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  image_render_queue *irq;
  obtain_image_render_queue(&global_data->render_thread->render_queue, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_CORNFLOWER_BLUE;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = td_portal->render_target.width;   // TODO
  irq->image_height = td_portal->render_target.height; // TODO
  irq->data.target_image.image_uid = td_portal->render_target.resource_uid;
  irq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  irq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(mc_node *) = (void (*)(mc_node *))child->layout->render_present;
      render_node_present(child);
    }
  }
}

void __mc_td_render_td_portal_present(image_render_queue *render_queue, mc_node *node)
{
  mc_td_portal *td_portal = (mc_td_portal *)node->data;

  mcr_issue_render_command_textured_quad(render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, td_portal->render_target.width,
                                         td_portal->render_target.height, td_portal->render_target.resource_uid);
}

void init_three_d_portal()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Node
  mc_node *node;
  mca_init_mc_node(global_data->global_node, NODE_TYPE_3D_PORTAL, &node);

  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  mca_init_node_layout(&node->layout);
  node->layout->preferred_width = 900;
  node->layout->preferred_height = 600;

  node->layout->padding.left = 400;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&__mc_td_determine_td_portal_extents;
  node->layout->update_layout = (void *)&__mc_td_update_td_portal_layout;
  node->layout->render_headless = (void *)&__mc_td_render_td_portal_headless;
  node->layout->render_present = (void *)&__mc_td_render_td_portal_present;

  mca_set_node_requires_layout_update(node);

  // Portal
  mc_td_portal *portal = (mc_td_portal *)malloc(sizeof(mc_td_portal));
  node->data = portal;
  portal->node = node;

  portal->render_target.resource_uid = 0;
  portal->render_target.width = node->layout->preferred_width;
  portal->render_target.height = node->layout->preferred_height;
  mcr_create_texture_resource(portal->render_target.width, portal->render_target.height, true,
                              &portal->render_target.resource_uid);
}