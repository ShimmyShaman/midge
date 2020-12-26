/* options_dialog.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/app_modules.h"
#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/ui_elements/ui_elements.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"

typedef struct mc_options_dialog_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;
  mcu_textblock *message_textblock;
  struct {
    unsigned int capacity, count, utilized;
    mcu_button **items;
  } displayed_items;

  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

  struct {
    void *state;
    void *result_delegate;
  } callback;

} mc_options_dialog_data;

void _mc_render_options_dialog_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_options_dialog_data *obd = (mc_options_dialog_data *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(render_thread_info *, mc_node *) =
          (void (*)(render_thread_info *, mc_node *))child->layout->render_headless;
      render_node_headless(render_thread, child);
    }
  }
}

void _mc_render_options_dialog_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_options_dialog_data *obd = (mc_options_dialog_data *)node->data;

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, obd->shade_color);

  // Children
  mca_render_typical_nodes_children_present(image_render_queue, node->children);
}

void _mc_handle_options_dialog_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mc_handle_options_dialog_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

// TODO -- all events app-wide need int returning success etc
void _mc_obd_item_selected(mci_input_event *input_event, mcu_button *button)
{
  mc_options_dialog_data *obd = (mc_options_dialog_data *)button->tag;

  // Activate Callback
  int (*result_delegate)(void *invoker_state, char *selected_folder) =
      (int (*)(void *, char *))obd->callback.result_delegate;
  if (result_delegate) {
    result_delegate(obd->callback.state, button->str->text);
  }
  obd->callback.state = NULL;
  obd->callback.result_delegate = NULL;

  // Wrap Up
  obd->node->layout->visible = false;
  mca_set_node_requires_rerender(obd->node);
}

int _mc_obd_on_options_dialog_request(void *handler_state, void *event_args)
{
  mc_options_dialog_data *obd = (mc_options_dialog_data *)handler_state;

  void **vary = (void **)event_args;
  char *message = (char *)vary[0];
  unsigned int option_count = *(unsigned int *)vary[1];
  const char **options = (const char **)vary[2];

  // Set Callback Info
  obd->callback.state = vary[3];
  obd->callback.result_delegate = vary[4];

  // Set the message
  MCcall(set_mc_str(obd->message_textblock->str, message == NULL ? "" : message));

  int a;
  mcu_button *b;
  for (a = 0; a < option_count && a < obd->displayed_items.count; ++a) {
    b = obd->displayed_items.items[a];

    MCcall(set_mc_str(b->str, options[a]));
    b->node->layout->visible = true;

    mca_set_node_requires_rerender(b->node);
  }
  for (; a < obd->displayed_items.count; ++a) {
    b = obd->displayed_items.items[a];

    b->node->layout->visible = false;
    mca_set_node_requires_rerender(b->node);
  }

  // Display
  obd->node->layout->visible = true;

  return 0;
}

int mc_obd_init_data(mc_node *module_node)
{
  mc_options_dialog_data *obd = (mc_options_dialog_data *)malloc(sizeof(mc_options_dialog_data));
  module_node->data = obd;
  obd->node = module_node;

  obd->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  obd->callback.state = NULL;
  obd->callback.result_delegate = NULL;

  obd->displayed_items.capacity = obd->displayed_items.count = 0U;

  return 0;
}

int mc_obd_init_ui(mc_node *module_node)
{
  mc_options_dialog_data *obd = (mc_options_dialog_data *)module_node->data;

  // Locals
  char buf[64];
  mca_node_layout *layout;
  mcu_button *button;

  // Panel
  MCcall(mcu_init_panel(module_node, &obd->panel));

  layout = obd->panel->node->layout;
  layout->max_width = 320;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  layout->max_height = 418;

  obd->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  // Message Block
  MCcall(mcu_init_textblock(obd->panel->node, &obd->message_textblock));

  layout = obd->message_textblock->node->layout;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  layout->preferred_width = 0.f;

  obd->message_textblock->background_color = COLOR_GRAPE;

  // Buttons to display items
  for (int a = 0; a < 10; ++a) {
    MCcall(mcu_init_button(obd->panel->node, &button));

    if (button->node->name) {
      free(button->node->name);
      button->node->name = NULL;
    }
    sprintf(buf, "options-dialog-item-button-%i", a);
    button->node->name = strdup(buf);

    button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    button->node->layout->padding = (mc_paddingf){6, 34 + 8 + a * 27, 6, 0};
    button->node->layout->max_width = 0U;
    button->node->layout->visible = false;

    button->tag = obd;
    button->left_click = (void *)&_mc_obd_item_selected;

    MCcall(set_mc_str(button->str, "button"));

    MCcall(append_to_collection((void ***)&obd->displayed_items.items, &obd->displayed_items.capacity,
                                &obd->displayed_items.count, button));
  }

  return 0;
}

int mc_obd_init_options_dialog(mc_node *app_root)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "options-dialog", &node));
  MCcall(mca_init_node_layout(&node->layout));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  node->layout->visible = false;
  node->layout->z_layer_index = 9;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_render_options_dialog_headless;
  node->layout->render_present = (void *)&_mc_render_options_dialog_present;
  node->layout->handle_input_event = (void *)&_mc_handle_options_dialog_input;

  MCcall(mc_obd_init_data(node));
  MCcall(mc_obd_init_ui(node));

  MCcall(
      mca_register_event_handler(MC_APP_EVENT_OPTIONS_DIALOG_REQUESTED, _mc_obd_on_options_dialog_request, node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  return 0;
}