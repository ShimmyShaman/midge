/* dropdown.c */

#include <stdio.h>
#include <stdlib.h>

#include "control/mc_controller.h"
#include "env/environment_definitions.h"

#include "modules/ui_elements/dropdown.h"

// void __mcu_determine_dropdown_extents(mc_node *node, layout_extent_restraints restraints)
// {
//   mcu_dropdown *dropdown = (mcu_dropdown *)node->data;

//   mc_rectf new_bounds = node->layout->__bounds;

//   float str_width, str_height;
//   if (!node->layout->preferred_width || !node->layout->preferred_height)
//     mcr_determine_text_display_dimensions(dropdown->font, dropdown->str->text, &str_width, &str_height);

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

// void __mcu_update_dropdown_layout(mc_node *node,mc_rectf const *available_area
// {
//   mcu_dropdown *dropdown = (mcu_dropdown *)node->data;

//   mc_rectf new_bounds = node->layout->__bounds;
//   new_bounds.x = available_area->x + node->layout->padding.left;
//   new_bounds.y = available_area->y + node->layout->padding.top;

//   // Determine if the new bounds is worth setting
//   if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
//       new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
//     node->layout->__bounds = new_bounds;
//     mca_set_node_requires_rerender(node);
//   }

//   node->layout->__requires_layout_update = false;

//   // Set rerender anyway because text could've changed
//   mca_set_node_requires_rerender(node);
// }

void __mcu_render_dropdown_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_dropdown *dropdown = (mcu_dropdown *)node->data;

  // Background
  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, dropdown->background_color);

  // Text
  // printf("renderdropdown- %u %u '%s' %s\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, dropdown->str->text, dropdown->font->name);
  mcr_issue_render_command_text(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                (unsigned int)node->layout->__bounds.y, dropdown->selected_str->text, dropdown->font,
                                dropdown->font_color);

  if (dropdown->options_extended) {
    // Dropdown Extended
    float box_height = 4 + 4 + dropdown->options.count * 24;

    // -- Background
    mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                          (unsigned int)(node->layout->__bounds.y + node->layout->__bounds.height),
                                          (unsigned int)node->layout->__bounds.width, box_height,
                                          dropdown->dropdown_shade);

    for (int a = 0; a < dropdown->options.count; ++a) {
      mcr_issue_render_command_text(
          image_render_queue, (unsigned int)node->layout->__bounds.x,
          (unsigned int)(node->layout->__bounds.y + node->layout->__bounds.height + 4 + a * 24),
          dropdown->selected_str->text, dropdown->font, dropdown->font_color);
    }
  }
}

void _mcu_dropdown_handle_input_event(mc_node *dropdown_node, mci_input_event *input_event)
{
  // printf("_mcu_dropdown_handle_input_event\n");
  mcu_dropdown *dropdown = (mcu_dropdown *)dropdown_node->data;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    // printf("_mcu_dropdown_handle_input_event-1\n");
    // if (dropdown->selection && (mc_mouse_button_code)input_event->button_code == MOUSE_BUTTON_LEFT) {
    //   // printf("_mcu_dropdown_handle_input_event-2\n");
    //   // Fire left-click
    //   // TODO fptr casting
    //   void (*selection)(mci_input_event *, mcu_dropdown *) =
    //       (void (*)(mci_input_event *, mcu_dropdown *))dropdown->selection;
    //   selection(input_event, dropdown);
    // }

    dropdown->options_extended = !dropdown->options_extended;
    mca_set_node_requires_rerender(dropdown_node);
  }

  input_event->handled = true;
}

int mcu_init_dropdown(mc_node *parent, mcu_dropdown **p_dropdown)
{
  // Node
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_DOESNT_MATTER, "unnamed-dropdown", &node));

  // printf("mcu_init_dropdown->node:%p name:%p '%s'-%i\n", node, node->name, node->name, strlen(node->name));

  // Layout
  MCcall(mca_init_node_layout(&node->layout));
  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&__mcu_render_dropdown_present;
  node->layout->handle_input_event = (void *)&_mcu_dropdown_handle_input_event;

  // Default Settings
  node->layout->min_width = 10;
  node->layout->min_height = 10;
  // node->layout->max_width = 140;
  node->layout->max_height = 26;

  // Control
  mcu_dropdown *dropdown = (mcu_dropdown *)malloc(sizeof(mcu_dropdown)); // TODO -- malloc check
  dropdown->node = node;
  node->data = dropdown;

  dropdown->options.capacity = dropdown->options.count = 0U;
  dropdown->options_extended = false;

  // printf("mcu-ib-3\n");
  dropdown->tag = NULL;
  dropdown->selection = NULL;

  MCcall(init_mc_str(&dropdown->selected_str));
  MCcall(set_mc_str(dropdown->selected_str, "dropdown"));
  dropdown->font = NULL;
  dropdown->font_color = COLOR_GHOST_WHITE;

  dropdown->background_color = COLOR_DIM_GRAY;
  dropdown->dropdown_shade = (render_color){0.35f, 0.35f, 0.35f, 0.95f};

  // Set to out pointer
  *p_dropdown = dropdown;

  MCcall(mca_attach_node_to_hierarchy(parent, node));

  return 0;
}

// void mca_init_dropdown_context_menu_options()
// {
//   // // TODO this void * casting business
//   // void *arg = (void *)&mca_visual_project_create_add_dropdown;
//   // mca_global_context_menu_add_option_to_node_context(NODE_TYPE_dropdown, "change text", arg);
// }