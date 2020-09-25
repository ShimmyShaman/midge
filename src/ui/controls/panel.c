#include "env/environment_definitions.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void __mui_determine_panel_extents(mc_node *node, layout_extent_restraints restraints)
{
  mui_panel *panel = (mui_panel *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->determine_layout_extents) {
      // TODO fptr casting
      void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
          (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
      determine_layout_extents(child, restraints);
    }
  }

  // Determine
  mc_rectf new_bounds = node->layout->__bounds;
  if (node->layout->preferred_width) {
    new_bounds.width = node->layout->preferred_width;
  }
  else {
    MCerror(7226, "NotYetSupported");
  }
  if (node->layout->preferred_height) {
    new_bounds.height = node->layout->preferred_height;
  }
  else {
    MCerror(7332, "NotYetSupported");
  }
}

void __mui_update_panel_layout(mc_node *node, mc_rectf *available_area)
{
  mui_panel *panel = (mui_panel *)node->data;

  mca_update_typical_node_layout(node, available_area);
  // mc_rectf new_bounds = node->layout->__bounds;
  // new_bounds.x = available_area->x + node->layout->padding.left;
  // new_bounds.y = available_area->y + node->layout->padding.top;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->update_layout) {
      // TODO fptr casting
      void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
      update_layout(child, &node->layout->__bounds);
    }
  }


  // Set rerender anyway because lazy TODO--maybe
  // mca_set_node_requires_rerender(node);
}

void __mui_render_panel_headless(mc_node *node)
{
  mui_panel *panel = (mui_panel *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }
}

void __mui_render_panel_present(image_render_queue *render_queue, mc_node *node)
{
  mui_panel *panel = (mui_panel *)node->data;

  mcr_issue_render_command_colored_quad(
      render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, panel->background_color);

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_presentation)(image_render_queue *, mc_node *) =
          (void (*)(image_render_queue *, mc_node *))child->layout->render_present;
      render_node_presentation(render_queue, child);
    }
  }
}

void mui_init_panel(mc_node *parent, mui_panel **p_panel)
{
  // Node
  mc_node *node;
  mca_init_mc_node(parent, NODE_TYPE_MUI_PANEL, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->determine_layout_extents = (void *)&__mui_determine_panel_extents;
  node->layout->update_layout = (void *)&__mui_update_panel_layout;
  node->layout->render_headless = (void *)&__mui_render_panel_headless;
  node->layout->render_present = (void *)&__mui_render_panel_present;

  // Control
  mui_panel *panel = (mui_panel *)malloc(sizeof(mui_panel));
  panel->node = node;
  node->data = panel;

  panel->background_color = COLOR_CORNFLOWER_BLUE;
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->alloc = 0;
  node->children->count = 0;

  // Set to out pointer
  *p_panel = panel;
}