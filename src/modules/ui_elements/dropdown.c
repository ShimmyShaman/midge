/* dropdown.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "control/mc_controller.h"
#include "env/environment_definitions.h"

#include "modules/ui_elements/button.h"
#include "modules/ui_elements/dropdown.h"

void _mcu_update_dropdown_extension_layout(mc_node *node, mc_rectf const *available_area)
{
  mcu_panel *extension = (mcu_panel *)node->data;
  mcu_dropdown *dropdown = (mcu_dropdown *)extension->tag;

  int a;
  mcu_button *button;
  mc_node *button_node;
  for (a = 0; a < extension->node->children->count && a < dropdown->options.count; ++a) {
    button_node = extension->node->children->items[a];
    button = (mcu_button *)button_node->data;

    button_node->layout->visible = true;
    mc_set_str(&button->str, dropdown->options.items[a]);
    mca_set_node_requires_rerender(button_node);
  }
  extension->node->layout->determined_extents.height = 2 + a * 27 + 2; // TODO...
  for (; a < extension->node->children->count; ++a) {
    button_node = extension->node->children->items[a];

    button_node->layout->visible = false;
  }

  mca_update_typical_node_layout(node, available_area);
}

// void _mcu_render_dropdown_extension_present(image_render_details *image_render_queue, mc_node *node)
// {
//   mcu_panel *panel = (mcu_panel *)node->data;

//   printf("rendering EXTENSION: %u %u %u %u\n", (unsigned int)node->layout->__bounds.x,
//          (unsigned int)node->layout->__bounds.y, (unsigned int)node->layout->__bounds.width,
//          (unsigned int)node->layout->__bounds.height);
//   // mcu_dropdown *dropdown = (mcu_dropdown *)node->data;

//   // Background
//   mcr_issue_render_command_colored_quad(
//       image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
//       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
//       panel->background_color);

//   // // Text
//   // // printf("renderdropdown- %u %u '%s' %s\n", (unsigned int)node->layout->__bounds.x,
//   // //        (unsigned int)node->layout->__bounds.y, dropdown->selected_str->text, dropdown->font);
//   // mcr_issue_render_command_text(image_render_queue, (unsigned int)node->layout->__bounds.x,
//   //                               (unsigned int)node->layout->__bounds.y,  NULL,dropdown->selected_str->text,
//   dropdown->font,
//   //                               dropdown->font_color);
// }

void _mcu_update_dropdown_node_layout(mc_node *node, mc_rectf const *available_area)
{
  // This method or most of its contents may not even be needed TODO
  // Don't update the child panel layout
  mca_update_typical_node_layout_partially(node, available_area, true, true, true, true, false);

  // Extension Panel
  mcu_dropdown *dropdown = (mcu_dropdown *)node->data;
  if (dropdown->options_extended) {
    // -- place it below
    mc_node *ext = dropdown->extension_panel->node;
    ext->layout->padding.left = node->layout->__bounds.x;
    ext->layout->padding.top = node->layout->__bounds.y + node->layout->__bounds.height;
    ext->layout->preferred_width = dropdown->node->layout->__bounds.width;
  }
}

// void _mcu_update_dropdown_extension_node_layout(mc_node *extension_node, mc_rectf const *available_area)
// {
//   // Nothing
// }

void _mcu_render_dropdown_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_dropdown *dropdown = (mcu_dropdown *)node->data;

  // Background
  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, dropdown->background_color);

  // Text
  // printf("renderdropdown- %u %u '%s' %s\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, dropdown->selected_str->text, dropdown->font);
  mcr_issue_render_command_text(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                (unsigned int)node->layout->__bounds.y, NULL, dropdown->selected_str->text,
                                dropdown->font, dropdown->font_color);
}

int _mcu_set_dropdown_extension_panel_visibility(mcu_dropdown *dropdown, bool visibility)
{
  // TODO -- maybe excess rerender calls made from this
  dropdown->options_extended = visibility;

  MCcall(mca_set_node_requires_rerender(dropdown->node));

  mc_node *extension_node = dropdown->extension_panel->node;
  if (visibility) {
    if (!extension_node->parent) {
      extension_node->layout->visible = true;

      // Set the appropriately higher z_layer_index
      mc_node *ancestor = dropdown->node;
      unsigned int z_layer = ancestor->layout->z_layer_index;
      while (ancestor->parent) {
        ancestor = ancestor->parent;
        z_layer = max(z_layer, ancestor->layout->z_layer_index);
      }
      extension_node->layout->z_layer_index = z_layer + 1U;

      // Attach
      MCcall(mca_attach_to_ancestor_root(ancestor, extension_node));
      MCcall(mca_focus_node(extension_node));
      MCcall(mca_set_node_requires_rerender(extension_node));
    }
  }
  else {
    extension_node->layout->visible = false;
    MCcall(mca_detach_from_parent(extension_node));
  }

  return 0;
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

    _mcu_set_dropdown_extension_panel_visibility(dropdown, !dropdown->options_extended);
  }

  input_event->handled = true;
}

void _mcu_dropdown_on_option_clicked(mci_input_event *input_event, mcu_button *button)
{
  mcu_dropdown *dropdown = button->tag;

  mc_set_str(dropdown->selected_str, button->str.text);

  // Propagate
  if (dropdown->selection) {
    // TODO fptr casting
    void (*selection_made)(mci_input_event *, mcu_dropdown *) =
        (void (*)(mci_input_event *, mcu_dropdown *))dropdown->selection;
    selection_made(input_event, dropdown);
  }

  _mcu_set_dropdown_extension_panel_visibility(dropdown, false);
  input_event->handled = true;
}

static void _mcu_dropdown_destroy_data(void *data)
{
  mcu_dropdown *dropdown = (mcu_dropdown *)data;

  // if (textblock->str.text)
  //   free(textblock->str.text);
  mc_release_str(dropdown->selected_str, true);

  free(data);
}

int mcu_init_dropdown(mc_node *parent, mcu_dropdown **p_dropdown)
{
  // Node
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_DOESNT_MATTER, "unnamed-dropdown", &node));
  node->destroy_data = (void *)&_mcu_dropdown_destroy_data;

  // printf("mcu_init_dropdown->node:%p name:%p '%s'-%i\n", node, node->name, node->name, strlen(node->name));

  // Layout
  MCcall(mca_init_node_layout(&node->layout));
  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&_mcu_update_dropdown_node_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&_mcu_render_dropdown_present;
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

  MCcall(mc_alloc_str(&dropdown->selected_str));
  dropdown->font = NULL;
  dropdown->font_color = COLOR_GHOST_WHITE;

  dropdown->background_color = COLOR_DIM_GRAY;

  dropdown->node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  dropdown->node->children->count = 0;
  dropdown->node->children->alloc = 0;

  // -- Panel
  MCcall(mcu_init_panel(NULL, &dropdown->extension_panel));

  dropdown->extension_panel->background_color = (render_color){0.15f, 0.15f, 0.25f, 0.92f};
  dropdown->extension_panel->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  dropdown->extension_panel->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  dropdown->extension_panel->node->layout->min_height = 18;
  dropdown->extension_panel->node->layout->max_height = 224;
  dropdown->extension_panel->tag = dropdown;

  dropdown->extension_panel->node->layout->update_layout = (void *)&_mcu_update_dropdown_extension_layout;
  // dropdown->extension_panel->node->layout->render_present = (void *)&_mcu_render_dropdown_extension_present;

  // Dropdown Options Buttons
  // It is assumed the only children of the extension panel are going to be buttons -- update the layout method if this
  // is ever not so
  char buf[64];
  mcu_button *button;
  for (int a = 0; a < 8; ++a) {
    MCcall(mcu_init_button(dropdown->extension_panel->node, &button));

    // button->node->layout->preferred_height = 26;
    button->node->layout->padding = (mc_paddingf){2, 2 + (26 + 1) * a, 2, 2};
    button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

    button->background_color = COLOR_TRANSPARENT;
    button->tag = dropdown;
    button->left_click = (void *)&_mcu_dropdown_on_option_clicked;
    MCcall(mc_set_str(&button->str, "button option"));

    sprintf(buf, "dropdown-option-button-%i", a);
    if (button->node->name)
      free(button->node->name);
    button->node->name = strdup(buf);
  }

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