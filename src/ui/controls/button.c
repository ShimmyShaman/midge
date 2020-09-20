#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void __mui_determine_button_extents(mc_node *node, layout_extent_restraints restraints)
{
  mui_button *button = (mui_button *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;

  float str_width, str_height;
  if (!node->layout->preferred_width || !node->layout->preferred_height)
    mcr_determine_text_display_dimensions(button->font_resource_uid, button->str->text, &str_width, &str_height);

  // Width
  if (node->layout->preferred_width)
    new_bounds.width = node->layout->preferred_width;
  else
    new_bounds.width = str_width;

  // Height
  if (node->layout->preferred_height)
    new_bounds.height = node->layout->preferred_height;
  else
    new_bounds.height = str_height;

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;
    mca_set_node_requires_layout_update(node);
  }
}

void __mui_update_button_layout(mc_node *node, mc_rectf *available_area)
{
  mui_button *button = (mui_button *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;
  new_bounds.x = available_area->x + node->layout->padding.left;
  new_bounds.y = available_area->y + node->layout->padding.top;

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;
    mca_set_node_requires_layout_update(node);
  }
}

void __mui_render_button_present(image_render_queue *render_queue, mc_node *node)
{
  mui_button *button = (mui_button *)node->data;

  // Background
  mcr_issue_render_command_colored_rect(render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, button->background_color);

  // Text
  // printf("renderbutton- %u %u %s %u\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, button->str->text, button->font_resource_uid);
  mcr_issue_render_command_text(render_queue, (unsigned int)node->layout->__bounds.x,
                                (unsigned int)node->layout->__bounds.y, button->str->text, button->font_resource_uid,
                                button->font_color);
}

void mui_init_button(mc_node *parent, mui_button **p_button)
{
  // Node
  mc_node *node;
  mca_init_mc_node(parent, NODE_TYPE_MUI_BUTTON, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->determine_layout_extents = (void *)&__mui_determine_button_extents;
  node->layout->update_layout = (void *)&__mui_update_button_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&__mui_render_button_present;

  // Control
  mui_button *button = (mui_button *)malloc(sizeof(mui_button));
  button->node = node;
  node->data = button;

  //   printf("mui-ib-3\n");
  button->tag = NULL;
  button->left_click = NULL;

  init_c_str(&button->str);
  set_c_str(button->str, "button");
  button->font_resource_uid = 0;
  button->font_color = COLOR_GHOST_WHITE;

  button->background_color = COLOR_DIM_GRAY;

  // Set to out pointer
  *p_button = button;
}