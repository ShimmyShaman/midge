/* textblock.c */

#include <stdio.h>
#include <stdlib.h>

#include "control/mc_controller.h"
#include "env/environment_definitions.h"

#include "modules/ui_elements/textblock.h"

void _mcu_determine_textblock_extents(mc_node *node, layout_extent_restraints restraints)
{
  mcu_textblock *textblock = (mcu_textblock *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;

  float str_width, str_height;
  if (!node->layout->preferred_width || !node->layout->preferred_height)
    mcr_determine_text_display_dimensions(textblock->font, textblock->str->text, &str_width, &str_height);

  // Width
  if (node->layout->preferred_width)
    new_bounds.width = node->layout->preferred_width;
  else {
    new_bounds.width = str_width;
  }

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

void _mcu_render_textblock_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_textblock *textblock = (mcu_textblock *)node->data;

  // Background
  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, textblock->background_color);

  // Text
  // printf("rendertextblock- %u %u %s %u\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, textblock->str->text, textblock->font->resource_uid);
  mcr_issue_render_command_text(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                (unsigned int)node->layout->__bounds.y, textblock->str->text, textblock->font,
                                textblock->font_color);
}

void _mcu_textblock_handle_input_event(mc_node *textblock_node, mci_input_event *input_event)
{
  // printf("_mcu_textblock_handle_input_event\n");
  mcu_textblock *textblock = (mcu_textblock *)textblock_node->data;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    // printf("_mcu_textblock_handle_input_event-1\n");
    if (textblock->left_click && (mc_mouse_button_code)input_event->button_code == MOUSE_BUTTON_LEFT) {
      // printf("_mcu_textblock_handle_input_event-2\n");
      // Fire left-click
      // TODO fptr casting
      void (*left_click)(mci_input_event *, mcu_textblock *) =
          (void (*)(mci_input_event *, mcu_textblock *))textblock->left_click;
      left_click(input_event, textblock);
    }
  }

  input_event->handled = true;
}

int mcu_init_textblock(mc_node *parent, mcu_textblock **p_textblock)
{
  // Node
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_DOESNT_MATTER, NULL, &node));

  // Layout
  MCcall(mca_init_node_layout(&node->layout));
  node->layout->determine_layout_extents = (void *)&_mcu_determine_textblock_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&_mcu_render_textblock_present;
  node->layout->handle_input_event = (void *)&_mcu_textblock_handle_input_event;

  // Default Settings
  node->layout->preferred_width = 80;
  node->layout->preferred_height = 24;

  // Control
  mcu_textblock *textblock = (mcu_textblock *)malloc(sizeof(mcu_textblock)); // TODO -- malloc check
  textblock->node = node;
  node->data = textblock;

  // printf("mcu-ib-3\n");
  textblock->tag = NULL;
  textblock->left_click = NULL;

  MCcall(init_mc_str(&textblock->str));
  MCcall(set_mc_str(textblock->str, "textblock"));
  textblock->font = NULL;
  textblock->font_color = COLOR_GHOST_WHITE;

  textblock->background_color = COLOR_DIM_GRAY;

  // Set to out pointer
  *p_textblock = textblock;

  MCcall(mca_attach_node_to_hierarchy(parent, node));

  return 0;
}

// void mca_init_textblock_context_menu_options()
// {
//   // // TODO this void * casting business
//   // void *arg = (void *)&mca_visual_project_create_add_textblock;
//   // mca_global_context_menu_add_option_to_node_context(NODE_TYPE_textblock, "change text", arg);
// }