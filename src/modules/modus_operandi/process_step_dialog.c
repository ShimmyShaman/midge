/* process_step_dialog.c */

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

#include "modules/modus_operandi/process_step_dialog.h"

void _mc_mocsd_render_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_process_step_dialog_data *psdd = (mc_process_step_dialog_data *)node->data;

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

void _mc_mocsd_render_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_process_step_dialog_data *psdd = (mc_process_step_dialog_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, psdd->shade_color);

  //   mcr_issue_render_command_colored_quad(
  //       image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       psdd->background_color);

  // Children
  mca_render_typical_nodes_children_present(image_render_queue, node->children);
}

void _mc_mocsd_handle_input(mc_node *node, mci_input_event *input_event)
{
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_mocsd_submit(mc_process_step_dialog_data *psdd)
{
  // Activate Callback
  int (*result_delegate)(void *invoker_state, char *selected_folder) =
      (int (*)(void *, char *))psdd->callback.result_delegate;
  if (result_delegate) {
    MCcall(result_delegate(psdd->callback.state, "TODO -- 84"));
  }
  psdd->callback.state = NULL;
  psdd->callback.result_delegate = NULL;

  // Wrap Up
  psdd->node->layout->visible = false;
  MCcall(mca_set_node_requires_rerender(psdd->node));

  return 0;
}

void _mc_mocsd_textbox_submit(mci_input_event *input_event, mcu_textbox *textbox)
{
  mc_process_step_dialog_data *psdd = (mc_process_step_dialog_data *)textbox->tag;

  _mc_mocsd_submit(psdd);
}

void _mc_mocsd_button_submit(mci_input_event *input_event, mcu_button *button)
{
  mc_process_step_dialog_data *psdd = (mc_process_step_dialog_data *)button->tag;

  // printf("_mc_mocsd_button_submit:'%s\n", psdd->current_directory->text);
  _mc_mocsd_submit(psdd);
}

int mc_mocsd_activate_process_step_dialog(mc_process_step_dialog_data *psdd, char *message, void *callback_state,
                                          void *callback_delegate)
{
  // Set Callback Info
  psdd->callback.state = callback_state;
  psdd->callback.result_delegate = callback_delegate;

  // Reset the type dropdown
  MCcall(set_mc_str(psdd->step_type_dropdown->selected_str, ""));
  MCcall(mca_set_node_requires_rerender(psdd->step_type_dropdown->node));

  // Set the message
  MCcall(set_mc_str(psdd->message_textblock->str, message == NULL ? "" : message));
  MCcall(mca_set_node_requires_rerender(psdd->message_textblock->node));

  // printf("_mc_mocsd_process_step_dialog_requested:tc'%s' %p %p '%s'\n", psdd->textbox->contents->text,
  //        psdd->textbox->contents->text, default_value, default_value);

  // Display
  psdd->node->layout->visible = true;

  return 0;
}

int _mc_mocsd_init_data(mc_node *module_node, mc_process_step_dialog_data **p_data)
{
  mc_process_step_dialog_data *psdd = (mc_process_step_dialog_data *)malloc(sizeof(mc_process_step_dialog_data));
  module_node->data = psdd;
  psdd->node = module_node;

  psdd->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  psdd->callback.state = NULL;
  psdd->callback.result_delegate = NULL;

  // mo_data->render_target.image = NULL;
  // mo_data->render_target.width = module_node->layout->preferred_width;
  // mo_data->render_target.height = module_node->layout->preferred_height;
  // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  //   while (!psdd->render_target.image) {
  //     // puts("wait");
  //     usleep(100);
  //   }

  *p_data = psdd;
  return 0;
}

int _mc_mocsd_init_ui(mc_node *module_node)
{
  mc_process_step_dialog_data *psdd = (mc_process_step_dialog_data *)module_node->data;

  // Locals
  char buf[64];
  mca_node_layout *layout;
  mcu_button *button;

  // Panel
  MCcall(mcu_init_panel(module_node, &psdd->panel));

  layout = psdd->panel->node->layout;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  // TODO -- set up extents override so the dialog can adjust to the size of the message
  layout->max_width = 400;
  layout->max_height = 280;

  psdd->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  // Message Block
  MCcall(mcu_init_textblock(psdd->panel->node, &psdd->message_textblock));

  layout = psdd->message_textblock->node->layout;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  layout->preferred_width = 0.f;

  psdd->message_textblock->background_color = COLOR_GRAPE;

  // Dropdown Button
  MCcall(mcu_init_dropdown(psdd->panel->node, &psdd->step_type_dropdown));

  layout = psdd->step_type_dropdown->node->layout;
  // layout->preferred_width = 64;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  // layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  // layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  psdd->step_type_dropdown->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(set_mc_str(psdd->step_type_dropdown->selected_str, "(none)"));
  psdd->step_type_dropdown->tag = psdd;
  // psdd->step_type_dropdown->selection = (void *)&_mc_mocsd_button_submit;

  // Complete Button
  MCcall(mcu_init_button(psdd->panel->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 64;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(set_mc_str(button->str, "Enter"));
  button->tag = psdd;
  button->left_click = (void *)&_mc_mocsd_button_submit;

  return 0;
}

int mc_mocsd_init_process_step_dialog(mc_node *app_root, mc_process_step_dialog_data **p_data)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // TODO -- get rid of node type

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "process-step-dialog", &node));
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
  node->layout->render_headless = (void *)&_mc_mocsd_render_headless;
  node->layout->render_present = (void *)&_mc_mocsd_render_present;
  node->layout->handle_input_event = (void *)&_mc_mocsd_handle_input;

  MCcall(_mc_mocsd_init_data(node, p_data));
  MCcall(_mc_mocsd_init_ui(node));

  // MCcall(mca_register_event_handler(MC_APP_EVENT_process_step_dialog_REQUESTED,
  // _mc_mocsd_process_step_dialog_requested,
  //                                   node->data));

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