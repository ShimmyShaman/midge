/* button.c */

#include <stdio.h>
#include <stdlib.h>

#include "control/mc_controller.h"
#include "env/environment_definitions.h"

#include "modules/ui_elements/button.h"

// void __mcu_determine_button_extents(mc_node *node, layout_extent_restraints restraints)
// {
//   mcu_button *button = (mcu_button *)node->data;

//   mc_rectf new_bounds = node->layout->__bounds;

//   float str_width, str_height;
//   if (!node->layout->preferred_width || !node->layout->preferred_height)
//     mcr_determine_text_display_dimensions(button->font, button->str.text, &str_width, &str_height);

//   // Width
//   if (node->layout->preferred_width)
//     new_bounds.width = node->layout->preferred_width;
//   else
//     new_bounds.width = str_width;

//   // Height
//   if (node->layout->preferred_height)
//     new_bounds.height = node->layout->preferred_height;
//   else
//     new_bounds.height = str_height;

//   // Determine if the new bounds is worth setting
//   if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
//       new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
//     node->layout->__bounds = new_bounds;
//     mca_set_node_requires_layout_update(node);
//   }
// }

void _mcu_update_button_layout(mc_node *node, mc_rectf const *available_area)
{
  mcu_button *button = (mcu_button *)node->data;

  mca_update_typical_node_layout(node, available_area);

  float text_width, text_height;
  mcr_determine_text_display_dimensions(button->font, button->str.text, &text_width, &text_height);

  // Align the button text
  switch (button->text_align.horizontal) {
  case HORIZONTAL_ALIGNMENT_LEFT:
    button->__state.text_offset_x = button->text_align.horizontal_margin;
    break;
  case HORIZONTAL_ALIGNMENT_CENTRED:
    button->__state.text_offset_x = max(0.f, (node->layout->__bounds.width - text_width) / 2);
    break;
  case HORIZONTAL_ALIGNMENT_RIGHT:
    button->__state.text_offset_x =
        max(0.f, node->layout->__bounds.width - text_width) - button->text_align.horizontal_margin;
    break;
  default:
    MCVerror(5877, "Not Supported : %i", button->text_align.horizontal);
  }
  switch (button->text_align.vertical) {
  case VERTICAL_ALIGNMENT_TOP:
    button->__state.text_offset_y = button->text_align.vertical_margin;
    break;
  case VERTICAL_ALIGNMENT_CENTRED:
    button->__state.text_offset_y = max(0.f, (node->layout->__bounds.height - text_height) / 2);
    break;
  case VERTICAL_ALIGNMENT_BOTTOM:
    button->__state.text_offset_y =
        max(0.f, node->layout->__bounds.height - text_height) - button->text_align.vertical_margin;
    break;
  default:
    MCVerror(5877, "Not Supported : %i", button->text_align.vertical);
  }

  // Set rerender anyway because text could've changed
  mca_set_node_requires_rerender(node);
}

void __mcu_render_button_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_button *button = (mcu_button *)node->data;

  // Background
  render_color color = button->background_color;
  if (button->enabled) {
    if (button->__state.highlighted) {
      color.r *= button->highlight_multiplier;
      color.g *= button->highlight_multiplier;
      color.b *= button->highlight_multiplier;
    }
  }
  else {
    color.r *= button->disabled_multiplier;
    color.g *= button->disabled_multiplier;
    color.b *= button->disabled_multiplier;
  }
  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, color);

  // Text
  color = button->font_color;
  if (!button->enabled) {
    color.r *= button->disabled_multiplier;
    color.g *= button->disabled_multiplier;
    color.b *= button->disabled_multiplier;
  }
  // printf("renderbutton- %u %u '%s' %s\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, button->str.text, button->font->name);
  mcr_issue_render_command_text(image_render_queue,
                                (unsigned int)(node->layout->__bounds.x + button->__state.text_offset_x),
                                (unsigned int)(node->layout->__bounds.y + button->__state.text_offset_y), NULL,
                                button->str.text, button->font, color);
}

void _mcu_button_handle_input_event(mc_node *button_node, mci_input_event *input_event)
{
  // printf("_mcu_button_handle_input_event\n");
  mcu_button *button = (mcu_button *)button_node->data;

  if (!button->enabled)
    return;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    // printf("_mcu_button_handle_input_event-1\n");
    if (button->left_click && (mc_mouse_button_code)input_event->button_code == MOUSE_BUTTON_LEFT) {
      // printf("_mcu_button_handle_input_event-2\n");
      // Fire left-click
      // TODO fptr casting
      // TODO int this delegate for error handling
      void (*left_click)(mci_input_event *, mcu_button *) =
          (void (*)(mci_input_event *, mcu_button *))button->left_click;
      // TODO -- MCcall(below) - have to make handle input event int
      left_click(input_event, button);
    }
  }

  input_event->handled = true;
}

static void _mcu_button_destroy_data(void *data)
{
  mcu_button *button = (mcu_button *)data;

  if (button->str.text)
    free(button->str.text);

  free(data);
}

int mcu_init_button(mc_node *parent, mcu_button **p_button)
{
  // Node
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_MCU_BUTTON, "unnamed-button", &node));
  node->destroy_data = (void *)&_mcu_button_destroy_data;

  // printf("mcu_init_button->node:%p name:%p '%s'-%i\n", node, node->name, node->name, strlen(node->name));

  // Layout
  MCcall(mca_init_node_layout(&node->layout));
  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&_mcu_update_button_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&__mcu_render_button_present;
  node->layout->handle_input_event = (void *)&_mcu_button_handle_input_event;

  // Default Settings
  node->layout->min_width = 10;
  node->layout->min_height = 10;
  // node->layout->max_width = 80;
  node->layout->max_height = 24;

  // Control
  mcu_button *button = (mcu_button *)malloc(sizeof(mcu_button)); // TODO -- malloc check
  button->node = node;
  node->data = button;

  // printf("mcu-ib-3\n");
  button->enabled = true;
  button->tag = NULL;
  button->left_click = NULL;

  MCcall(mc_init_str(&button->str, 8));
  MCcall(mc_set_str(&button->str, "button"));
  button->font = NULL;
  button->font_color = COLOR_GHOST_WHITE;

  button->text_align.horizontal = HORIZONTAL_ALIGNMENT_CENTRED;
  button->text_align.vertical = VERTICAL_ALIGNMENT_CENTRED;
  button->text_align.horizontal_margin = 2;
  button->text_align.vertical_margin = 2;

  button->background_color = COLOR_DIM_GRAY;
  button->disabled_multiplier = 0.7f;
  button->highlight_multiplier = 1.43;

  button->__state.highlighted = false;

  // Set to out pointer
  *p_button = button;

  MCcall(mca_attach_node_to_hierarchy(parent, node));

  return 0;
}

// void mcu_destroy_button(mcu_button *button)
// {
//   // Destroy Node from Hierarchy
//   if (button->node) {
//     mca_destroy_mc_node(button->node);
//     button->node = NULL;
//   }

// }