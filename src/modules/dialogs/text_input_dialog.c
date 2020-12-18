/* text_input_dialog.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/app_modules.h"
#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/mc_io/mc_io.h"
#include "modules/ui_elements/ui_elements.h"

typedef struct mc_text_input_dialog_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;
  mcu_textblock *message_textblock;
  mcu_textbox *textbox;

  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

  struct {
    void *state;
    void *result_delegate;
  } callback;

} mc_text_input_dialog_data;

void mc_tid_render_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_text_input_dialog_data *tid = (mc_text_input_dialog_data *)node->data;

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

  // // Render the render target
  // midge_app_info *app_info;
  // mc_obtain_midge_app_info(&app_info);

  // // Children
  // for (int a = 0; a < node->children->count; ++a) {
  //   mc_node *child = node->children->items[a];
  //   if (child->layout && child->layout->visible && child->layout->render_present) {
  //     // TODO fptr casting
  //     void (*render_node_present)(image_render_details *, mc_node *) =
  //         (void (*)(image_render_details *, mc_node *))child->layout->render_present;
  //     render_node_present(irq, child);
  //   }
  // }

  // mcr_submit_image_render_request(app_info->render_thread, irq);
}

void mc_tid_render_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_text_input_dialog_data *tid = (mc_text_input_dialog_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, tid->shade_color);

  //   mcr_issue_render_command_colored_quad(
  //       image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       tid->background_color);

  // Children
  mca_render_typical_nodes_children_present(image_render_queue, node->children);
}

void mc_tid_handle_input(mc_node *node, mci_input_event *input_event)
{
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_tid_submit(mc_text_input_dialog_data *tid)
{
  // Activate Callback
  int (*result_delegate)(void *invoker_state, char *selected_folder) =
      (int (*)(void *, char *))tid->callback.result_delegate;
  if (result_delegate) {
    MCcall(result_delegate(tid->callback.state, tid->textbox->contents->text));
  }
  tid->callback.state = NULL;
  tid->callback.result_delegate = NULL;

  // Wrap Up
  tid->node->layout->visible = false;
  MCcall(mca_set_node_requires_rerender(tid->node));

  return 0;
}

void _mc_tid_textbox_submit(mci_input_event *input_event, mcu_textbox *textbox)
{
  mc_text_input_dialog_data *tid = (mc_text_input_dialog_data *)textbox->tag;

  _mc_tid_submit(tid);
}

void _mc_tid_button_submit(mci_input_event *input_event, mcu_button *button)
{
  mc_text_input_dialog_data *tid = (mc_text_input_dialog_data *)button->tag;

  // printf("_mc_tid_button_submit:'%s\n", tid->current_directory->text);
  _mc_tid_submit(tid);
}

int _mc_tid_text_input_dialog_requested(void *handler_state, void *event_args)
{
  mc_text_input_dialog_data *tid = (mc_text_input_dialog_data *)handler_state;

  void **vary = (void **)event_args;
  char *message = (char *)vary[0];
  char *default_value = (char *)vary[1];

  // Set Callback Info
  tid->callback.state = vary[2];
  tid->callback.result_delegate = vary[3];

  // Reset the textbox
  MCcall(set_mc_str(tid->textbox->contents, default_value ? default_value : ""));

  // Set the message
  MCcall(set_mc_str(tid->message_textblock->str,  message == NULL ? "" : message));

  // printf("_mc_tid_text_input_dialog_requested:tc'%s' %p %p '%s'\n", tid->textbox->contents->text,
  //        tid->textbox->contents->text, default_value, default_value);

  // Display
  tid->node->layout->visible = true;

  MCcall(mca_set_node_requires_rerender(tid->textbox->node));
  MCcall(mca_set_node_requires_layout_update(tid->message_textblock->node));

  MCcall(mca_focus_node(tid->textbox->node));

  return 0;
}

int _mc_tid_init_data(mc_node *module_node)
{
  mc_text_input_dialog_data *tid = (mc_text_input_dialog_data *)malloc(sizeof(mc_text_input_dialog_data));
  module_node->data = tid;
  tid->node = module_node;

  tid->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  tid->callback.state = NULL;
  tid->callback.result_delegate = NULL;

  // mo_data->render_target.image = NULL;
  // mo_data->render_target.width = module_node->layout->preferred_width;
  // mo_data->render_target.height = module_node->layout->preferred_height;
  // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  //   while (!tid->render_target.image) {
  //     // puts("wait");
  //     usleep(100);
  //   }

  return 0;
}

int mc_tid_init_ui(mc_node *module_node)
{
  mc_text_input_dialog_data *tid = (mc_text_input_dialog_data *)module_node->data;

  // Locals
  char buf[64];
  mca_node_layout *layout;
  mcu_button *button;

  // Panel
  MCcall(mcu_init_panel(module_node, &tid->panel));

  layout = tid->panel->node->layout;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  // TODO -- set up extents override so the dialog can adjust to the size of the message
  layout->max_width = 400;
  layout->max_height = 62;

  tid->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  // Message Block
  MCcall(mcu_init_textblock(tid->panel->node, &tid->message_textblock));

  layout = tid->message_textblock->node->layout;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  layout->preferred_width = 0.f;

  tid->message_textblock->background_color = COLOR_GRAPE;

  // Textbox
  MCcall(mcu_init_textbox(tid->panel->node, &tid->textbox));

  layout = tid->textbox->node->layout;
  layout->preferred_width = 320;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  tid->textbox->tag = tid;
  tid->textbox->submit = (void *)_mc_tid_textbox_submit;

  // Complete Button
  MCcall(mcu_init_button(tid->panel->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 64;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(set_mc_str(button->str, "Enter"));
  button->tag = tid;
  button->left_click = (void *)&_mc_tid_button_submit;

  return 0;
}

int mc_tid_init_text_input_dialog(mc_node *app_root)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // TODO -- get rid of node type

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "text-input-dialog", &node));
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
  node->layout->render_headless = (void *)&mc_tid_render_headless;
  node->layout->render_present = (void *)&mc_tid_render_present;
  node->layout->handle_input_event = (void *)&mc_tid_handle_input;

  MCcall(_mc_tid_init_data(node));
  MCcall(mc_tid_init_ui(node));

  MCcall(mca_register_event_handler(MC_APP_EVENT_TEXT_INPUT_DIALOG_REQUESTED, _mc_tid_text_input_dialog_requested,
                                    node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  // Panel
  // mcu_panel *panel;
  // mcu_init_panel(app_info->global_node, &panel);

  // panel->node->layout->padding = {300, 400, 800, 60};
  // panel->background_color = COLOR_GREEN;

  // mcu_set_element_update(panel->element);

  // Text Block
  // mcu_text_block *text_block;
  // mcu_init_text_block(app_info->global_node, &text_block);

  // mca_node_layout *layout = text_block->element->layout;
  // layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  // layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  // layout->padding = {150, 300, 0, 0};

  // set_mc_str(text_block->str, "");
  // for (int a = 32; a < 128; ++a) {
  //   char buf[2];
  //   buf[0] = (char)a;
  //   buf[1] = '\0';
  //   append_to_mc_str(text_block->str, buf);
  // }
  // text_block->font_color = COLOR_LIGHT_YELLOW;

  return 0;
}