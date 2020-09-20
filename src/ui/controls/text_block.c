#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void __mui_determine_text_block_extents(mc_node *node, layout_extent_restraints restraints)
{
  mui_text_block *text_block = (mui_text_block *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;

  float str_width, str_height;
  if (!node->layout->preferred_width || !node->layout->preferred_height)
    mcr_determine_text_display_dimensions(text_block->font_resource_uid, text_block->str->text, &str_width,
                                          &str_height);

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

void __mui_update_text_block_layout(mc_node *node, mc_rectf *available_area)
{
  mui_text_block *text_block = (mui_text_block *)node->data;

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

void __mui_render_text_block_present(image_render_queue *render_queue, mc_node *node)
{
  mui_text_block *text_block = (mui_text_block *)node->data;

  // Text
  // printf("rendertext_block- %u %u %s %u\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, text_block->str->text, text_block->font_resource_uid);
  mcr_issue_render_command_text(render_queue, (unsigned int)node->layout->__bounds.x,
                                (unsigned int)node->layout->__bounds.y, text_block->str->text,
                                text_block->font_resource_uid, text_block->font_color);
}

void mui_init_text_block(mc_node *parent, mui_text_block **p_text_block)
{
  // Node
  mc_node *node;
  mca_init_mc_node(parent, NODE_TYPE_MUI_TEXT_BLOCK, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->determine_layout_extents = (void *)&__mui_determine_text_block_extents;
  node->layout->update_layout = (void *)&__mui_update_text_block_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&__mui_render_text_block_present;

  // Control
  mui_text_block *text_block = (mui_text_block *)malloc(sizeof(mui_text_block));
  text_block->node = node;
  node->data = text_block;

  init_c_str(&text_block->str);
  text_block->font_resource_uid = 0;
  text_block->font_color = COLOR_GHOST_WHITE;

  // Set to out pointer
  *p_text_block = text_block;
}