#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void __mcu_determine_text_block_extents(mc_node *node, layout_extent_restraints restraints)
{
  mcu_text_block *text_block = (mcu_text_block *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;

  float str_width, str_height;
  if (!node->layout->preferred_width || !node->layout->preferred_height)
    mcr_determine_text_display_dimensions(text_block->font, text_block->str->text, &str_width, &str_height);

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

void __mcu_update_text_block_layout(mc_node *node, mc_rectf *available_area)
{
  mcu_text_block *text_block = (mcu_text_block *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;
  new_bounds.x = available_area->x + node->layout->padding.left;
  new_bounds.y = available_area->y + node->layout->padding.top;

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;
    mca_set_node_requires_rerender(node);
  }

  node->layout->__requires_layout_update = false;

  // Set rerender anyway because lazy TODO--maybe
  mca_set_node_requires_rerender(node);
}

void __mcu_render_text_block_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_text_block *text_block = (mcu_text_block *)node->data;

  // Text
  // printf("rendertext_block- %u %u %s %u\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, text_block->str->text, text_block->font->resource_uid);
  mcr_issue_render_command_text(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                (unsigned int)node->layout->__bounds.y, text_block->str->text, text_block->font,
                                text_block->font_color);
}

void mcu_init_text_block(mc_node *parent, mcu_text_block **p_text_block)
{
  // // Node
  // mc_node *node;
  // mca_init_mc_node(parent, NODE_TYPE_MCU_TEXT_BLOCK, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->determine_layout_extents = (void *)&__mcu_determine_text_block_extents;
  node->layout->update_layout = (void *)&__mcu_update_text_block_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&__mcu_render_text_block_present;

  // Control
  mcu_text_block *text_block = (mcu_text_block *)malloc(sizeof(mcu_text_block));
  text_block->node = node;
  node->data = text_block;

  init_mc_str(&text_block->str);
  text_block->font = NULL;
  text_block->font_color = COLOR_GHOST_WHITE;

  // Set to out pointer
  *p_text_block = text_block;
}