
#include "core/core_definitions.h"
#include "modules/app_modules.h"
#include "render/render_common.h"

void __mcm_determine_function_editor_extents(mc_node *node, layout_extent_restraints restraints)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)node->data;

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
    // MCerror(7295, "NotYetSupported");
  }
  if (node->layout->preferred_height) {
    new_bounds.height = node->layout->preferred_height;
  }
  else {
    MCerror(7301, "NotYetSupported");
  }

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;
    mca_set_node_requires_layout_update(node);
  }
}

void __mcm_update_function_editor_layout(mc_node *node, mc_rectf *available_area)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;
  new_bounds.x = available_area->x + node->layout->padding.left;
  new_bounds.y = available_area->y + node->layout->padding.top;
  new_bounds.width = available_area->width - node->layout->padding.left - node->layout->padding.right;

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;

    mca_set_node_requires_rerender(node);
  }

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->update_layout) {
      // TODO fptr casting
      void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
      update_layout(child, &node->layout->__bounds);
    }
  }

  node->layout->__requires_layout_update = false;

  // Set rerender anyway because lazy TODO--maybe
  mca_set_node_requires_rerender(node);
}

void __mcm_render_function_editor_headless(mc_node *node)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)node->data;

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

void __mcm_render_function_editor_present(image_render_queue *render_queue, mc_node *node)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)node->data;

  mcr_issue_render_command_colored_rect(render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, function_editor->background_color);
  //   printf("__mcm_render_function_editor_present %u %u %u %u\n", (unsigned int)node->layout->__bounds.x,
  //          (unsigned int)node->layout->__bounds.y, (unsigned int)node->layout->__bounds.width,
  //          (unsigned int)node->layout->__bounds.height);

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

void mcm_init_function_editor(mc_node *parent_node, mcm_function_editor **p_function_editor)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)malloc(sizeof(mcm_function_editor));
  mca_init_mc_node(parent_node, NODE_TYPE_FUNCTION_EDITOR, &function_editor->node);
  function_editor->node->data = function_editor;

  // Layout
  mca_init_node_layout(&function_editor->node->layout);
  mca_node_layout *layout = function_editor->node->layout;
  layout->determine_layout_extents = (void *)&__mcm_determine_function_editor_extents;
  layout->update_layout = (void *)&__mcm_update_function_editor_layout;
  layout->render_headless = (void *)&__mcm_render_function_editor_headless;
  layout->render_present = (void *)&__mcm_render_function_editor_present;

  // layout->preferred_width = 980;
  layout->preferred_height = 720;
  layout->padding.left = 400;
  layout->padding.top = 20;
  layout->padding.right = 20;

  function_editor->background_color = COLOR_NEARLY_BLACK;
  function_editor->node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  function_editor->node->children->alloc = 0;
  function_editor->node->children->count = 0;

  *p_function_editor = function_editor;
}