/* create_process_dialog.c */

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

#include "modules/modus_operandi/create_process_dialog.h"
#include "modules/modus_operandi/mo_types.h"

void _mc_mo_cpd_render_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)node->data;

  // Children
  mca_render_typical_nodes_children_headless(render_thread, node->children);

  // mcr_submit_image_render_request(app_info->render_thread, irq);
}

void _mc_mo_cpd_render_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)node->data;

  // printf("_mc_mo_cpd_render_present : %s\n", node->layout->visible ? "visible" : "not-visible");

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, cpd->shade_color);

  //   mcr_issue_render_command_colored_quad(
  //       image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       cpd->background_color);

  // Children
  mca_render_typical_nodes_children_present(image_render_queue, node->children);
}

void _mc_mo_handle_cpd_input(mc_node *node, mci_input_event *input_event)
{
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);

    input_event->handled = true;
  }
}

int mc_mo_close_create_process_dialog(mc_create_process_dialog_data *cpd)
{
  cpd->callback.state = NULL;
  cpd->callback.result_delegate = NULL;

  // Reset state
  // if (cpd->active_options_panel) {
  //   cpd->active_options_panel->node->layout->visible = false;
  //   cpd->active_options_panel = NULL;
  // }

  // Wrap Up
  cpd->node->layout->visible = false;
  MCcall(mca_set_node_requires_rerender(cpd->node));

  return 0;
}

int _mc_mo_cpd_submit(mc_create_process_dialog_data *cpd)
{
  // Activate Callback
  int (*result_delegate)(void *invoker_state, void *created_process) =
      (int (*)(void *, void *))cpd->callback.result_delegate;
  if (result_delegate) {
    MCcall(result_delegate(cpd->callback.state, "TODO -- 84"));
  }

  MCcall(mc_mo_close_create_process_dialog(cpd));

  return 0;
}

// void _mc_mo_textbox_submit(mci_input_event *input_event, mcu_textbox *textbox)
// {
//   mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)textbox->tag;

//   _mc_mo_submit(cpd);
// }

void _mc_mo_cpd_cancel_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_mo_close_create_process_dialog((mc_create_process_dialog_data *)button->tag);
}

int mc_mo_activate_create_process_dialog(mc_create_process_dialog_data *cpd, void *callback_state,
                                         void *callback_delegate)
{
  // Set Callback Info
  cpd->callback.state = callback_state;
  cpd->callback.result_delegate = callback_delegate;

  // Display
  // puts("mc_mo_activate_create_process_dialog");
  cpd->node->layout->visible = true;
  MCcall(mca_set_node_requires_rerender(cpd->node));

  return 0;
}

////////////////////////////////////////////////////////////
////////////////      Initialization      //////////////////
////////////////////////////////////////////////////////////

int _mc_mo_cpd_init_data(mc_node *module_node, mc_create_process_dialog_data **p_data)
{
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)malloc(sizeof(mc_create_process_dialog_data));
  module_node->data = cpd;
  cpd->node = module_node;

  cpd->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  cpd->callback.state = NULL;
  cpd->callback.result_delegate = NULL;

  cpd->cells.size = 32;
  cpd->cells.ary = (mc_mo_cpd_step_data *)calloc(cpd->cells.size, sizeof(mc_mo_cpd_step_data));

  // DEBUG
  cpd->cells.ary[0].type = MO_STEP_ABSTRACT_TITLE;

  // mo_data->render_target.image = NULL;
  // mo_data->render_target.width = module_node->layout->preferred_width;
  // mo_data->render_target.height = module_node->layout->preferred_height;
  // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  //   while (!cpd->render_target.image) {
  //     // puts("wait");
  //     usleep(100);
  //   }

  *p_data = cpd;
  return 0;
}

int _mc_mo_init_cpd_ui(mc_node *module_node)
{
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)module_node->data;

  // Locals
  char buf[64];
  mca_node_layout *layout;
  mcu_button *button;

  // Panel
  MCcall(mcu_init_panel(module_node, &cpd->panel));

  layout = cpd->panel->node->layout;
  layout->padding = (mc_paddingf){60, 40, 60, 40};
  // TODO -- set up extents override so the dialog can adjust to the size of the message
  // layout->max_width = 520;
  // layout->max_height = 360;

  cpd->panel->background_color = (render_color){0.28f, 0.28f, 0.21f, 1.f};

  // Exit Button
  MCcall(mcu_init_button(cpd->panel->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 16;
  layout->preferred_height = 16;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(set_mc_str(button->str, "X"));
  button->tag = cpd;
  button->left_click = (void *)&_mc_mo_cpd_cancel_clicked;

  return 0;
}

int mc_mo_init_create_process_dialog(mc_node *app_root, mc_create_process_dialog_data **p_data)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // TODO -- get rid of node type

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "create-process-dialog", &node));
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
  node->layout->render_headless = (void *)&_mc_mo_cpd_render_headless;
  node->layout->render_present = (void *)&_mc_mo_cpd_render_present;
  node->layout->handle_input_event = (void *)&_mc_mo_handle_cpd_input;

  MCcall(_mc_mo_cpd_init_data(node, p_data));
  MCcall(_mc_mo_init_cpd_ui(node));

  // MCcall(mca_register_event_handler(MC_APP_EVENT_create_process_dialog_REQUESTED,
  // _mc_mo_create_process_dialog_requested,
  //                                   node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  return 0;
}