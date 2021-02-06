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
  bool text_determined = false;

  const float MAX_EXTENT_VALUE = 100000.f;
  mca_node_layout *layout = node->layout;

  // Width
  if (layout->preferred_width) {
    // Set to preferred width
    layout->determined_extents.width = layout->preferred_width;
  }
  else {
    mcr_determine_text_display_dimensions(textblock->font, textblock->str->text, &str_width, &str_height);
    text_determined = true;

    layout->determined_extents.width = str_width;

    // Specified bounds
    if (layout->min_width && layout->determined_extents.width < layout->min_width) {
      layout->determined_extents.width = layout->min_width;
    }
    if (layout->max_width && layout->determined_extents.width > layout->max_width) {
      layout->determined_extents.width = layout->max_width;
    }

    if (layout->determined_extents.width < 0) {
      layout->determined_extents.width = 0;
    }
  }

  // Height
  if (layout->preferred_height) {
    // Set to preferred height
    layout->determined_extents.height = layout->preferred_height;
  }
  else {
    if (!text_determined) {
      mcr_determine_text_display_dimensions(textblock->font, textblock->str->text, &str_width, &str_height);
    }

    layout->determined_extents.height = str_height;

    // Specified bounds
    if (layout->min_height && layout->determined_extents.height < layout->min_height) {
      layout->determined_extents.height = layout->min_height;
    }
    if (layout->max_height && layout->determined_extents.height > layout->max_height) {
      layout->determined_extents.height = layout->max_height;
    }

    if (layout->determined_extents.height < 0) {
      layout->determined_extents.height = 0;
    }
  }
}

void _mcu_render_textblock_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_textblock *textblock = (mcu_textblock *)node->data;

  // printf("rendertextblock- %u %u %s\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, textblock->str->text);

  // Background
  if (textblock->background_color.a) {
    mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                          (unsigned int)node->layout->__bounds.y,
                                          (unsigned int)node->layout->__bounds.width,
                                          (unsigned int)node->layout->__bounds.height, textblock->background_color);
  }

  // Text
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

      input_event->handled = true;
    }
  }
}

int mcu_init_textblock(mc_node *parent, mcu_textblock **p_textblock)
{
  // Node
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_DOESNT_MATTER, "unnamed-textblock", &node));

  // Layout
  MCcall(mca_init_node_layout(&node->layout));
  node->layout->determine_layout_extents = (void *)&_mcu_determine_textblock_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&_mcu_render_textblock_present;
  node->layout->handle_input_event = (void *)&_mcu_textblock_handle_input_event;

  // Default Settings
  node->layout->min_width = 10;
  node->layout->min_height = 24;

  // Control
  mcu_textblock *textblock = (mcu_textblock *)malloc(sizeof(mcu_textblock)); // TODO -- malloc check
  textblock->node = node;
  node->data = textblock;

  // printf("mcu-ib-3\n");
  textblock->tag = NULL;
  textblock->left_click = NULL; // TODO -- remove this textblocks shouldn't be able to be clicked? they're not buttons

  MCcall(mc_alloc_str(&textblock->str));
  MCcall(mc_set_str(textblock->str, "textblock"));
  textblock->font = NULL;
  textblock->font_color = COLOR_GHOST_WHITE;

  textblock->background_color = COLOR_TRANSPARENT;

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