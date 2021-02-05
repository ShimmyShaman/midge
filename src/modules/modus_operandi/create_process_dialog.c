/* create_process_dialog.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/app_modules.h"
#include "core/midge_app.h"
#include "mc_error_handling.h"
#include "render/render_common.h"

#include "modules/ui_elements/ui_elements.h"

#include "modules/modus_operandi/create_process_dialog.h"
#include "modules/modus_operandi/mo_types.h"

#define MO_STEP_PROCESS_TITLE 1000
#define MO_STEP_UNDESIGNATED 1001
#define MO_STEP_CONTEXT_PARAMETER 1002

#define MO_CPD_CELL_TEXTBLOCK_COUNT 3

void _mc_mo_cpd_render_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)node->data;

  // Children
  mca_render_node_list_headless(render_thread, node->children);

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
  mca_render_node_list_present(image_render_queue, node->children);

  // mc_mo_cpd_step_data *cell;
  // for (cell = cpd->cells.ary; cell < &cpd->cells.ary[cpd->cells.size]; ++cell) {

  //   if (!cell->panel->node->layout->visible)
  //     break;

  //   printf("cell %i %i: %.2f %.2f %.2f %.2f\n", cell - cpd->cells.ary, cell->type,
  //          cell->panel->node->layout->__bounds.x, cell->panel->node->layout->__bounds.y,
  //          cell->panel->node->layout->__bounds.width, cell->panel->node->layout->__bounds.height);
  // }
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
  mc_mo_cpd_step_data *cell;
  for (cell = cpd->cells.ary; cell < &cpd->cells.ary[cpd->cells.size]; ++cell) {
    cell->type = MO_STEP_NULL;
    cell->panel->node->layout->visible = false;
  }

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

int _mc_mo_cpd_set_cell(mc_mo_cpd_step_data *cell, mo_op_step_action_type type)
{
  int a;

  // Set type & visibility
  cell->type = type;

  mca_node_layout *layout = cell->panel->node->layout;
  layout->visible = true;

  cell->dropdown->node->layout->visible = false;
  cell->delegate_button->node->layout->visible = false;
  cell->textbox->node->layout->visible = false;
  for (a = 0; a < MO_CPD_CELL_TEXTBLOCK_COUNT; ++a)
    cell->textblocks[a]->node->layout->visible = false;

  switch (type) {
  case MO_STEP_PROCESS_TITLE: {
    layout->preferred_width = 240;
    layout->preferred_height = 60;

    cell->panel->background_color = (render_color){0.16f, 0.16f, 0.12f, 1.f};

    // Textbox
    layout = cell->textbox->node->layout;
    layout->visible = true;
    layout->padding = (mc_paddingf){4, 4, 4, 4};

    MCcall(set_mc_str(cell->textbox->contents, "process-title"));
    cell->textbox->cursor.col = strlen("process-title");

    // Layout Updates
    MCcall(mca_set_node_requires_layout_update(cell->textbox->node));
  } break;
  case MO_STEP_UNDESIGNATED: {
    layout->preferred_width = 240;
    layout->preferred_height = 60;

    cell->panel->background_color = (render_color){0.19f, 0.03f, 0.18f, 1.f};

    // Dropdown
    cell->dropdown->options.count = 0U;
    MCcall(append_to_collection((void ***)&cell->dropdown->options.items, &cell->dropdown->options.capacity,
                                &cell->dropdown->options.count, "Parameter"));

    layout = cell->dropdown->node->layout;
    layout->visible = true;

    // Layout Updates
    MCcall(mca_set_node_requires_layout_update(cell->dropdown->node));
  } break;
  case MO_STEP_CONTEXT_PARAMETER: {
    layout->preferred_width = 320;
    layout->preferred_height = 96;

    cell->panel->background_color = (render_color){0.04f, 0.24f, 0.06f, 1.f};

    // Title
    MCcall(set_mc_str(cell->textblocks[0]->str, "Context Parameter"));

    layout = cell->textblocks[0]->node->layout;
    layout->visible = true;
    layout->padding = (mc_paddingf){4, 4, 4, 4};
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;

    // Param Label
    MCcall(set_mc_str(cell->textblocks[1]->str, "key:"));

    layout = cell->textblocks[1]->node->layout;
    layout->visible = true;
    layout->padding = (mc_paddingf){4, 28, 4, 4};
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;

    // Textbox
    MCcall(set_mc_str(cell->textbox->contents, ""));
    cell->textbox->cursor.col = strlen("");

    layout = cell->textbox->node->layout;
    layout->visible = true;
    layout->padding = (mc_paddingf){42, 28, 4, 4};

    // Obtain Delegate
    layout = cell->delegate_button->node->layout;
    layout->visible = true;
    layout->padding = (mc_paddingf){4, 56, 4, 4};

    // Layout Updates
    MCcall(mca_set_node_requires_layout_update(cell->textblocks[0]->node));
    MCcall(mca_set_node_requires_layout_update(cell->textblocks[1]->node));
    MCcall(mca_set_node_requires_layout_update(cell->textblocks[2]->node));
    MCcall(mca_set_node_requires_layout_update(cell->delegate_button->node));
    MCcall(mca_set_node_requires_layout_update(cell->textbox->node));
  } break;
  default:
    MCerror(7892, "TODO : %i", type);
    break;
  }

  return 0;
}

int _mc_mo_cpd_create_new_cell(mc_create_process_dialog_data *cpd, mo_op_step_action_type type, float offX, float offY,
                               mc_mo_cpd_step_data **dest)
{
  mc_mo_cpd_step_data *cell;
  // Obtain the new cell ptr
  int a;
  for (a = 0; a < cpd->cells.size; ++a) {
    if (!cpd->cells.ary[a].type)
      break;
  }
  if (a >= cpd->cells.size) {
    MCerror(7293, "TODO Cells size too big");
  }
  cell = &cpd->cells.ary[a];

  printf("new cell @ %p\n", cell);
  cell->panel->node->layout->padding = (mc_paddingf){offX, offY, 0, 0};

  MCcall(_mc_mo_cpd_set_cell(cell, type));

  if (dest)
    *dest = cell;

  return 0;
}

void _mc_mo_cpd_cell_dropdown_selection(mci_input_event *input_event, mcu_dropdown *dropdown)
{
  mc_mo_cpd_step_data *cell = dropdown->tag;
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)cell->panel->node->parent->parent->data;

  if (!strcmp(dropdown->selected_str->text, "Parameter")) {
    _mc_mo_cpd_set_cell(cell, MO_STEP_CONTEXT_PARAMETER);
  }
  else {
    printf("5928 ERROR - unhandled dropdown selection:'%s'\n", dropdown->selected_str->text);
  }
}

void _mc_mo_cpd_cell_delegate_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_mo_cpd_step_data *cell = button->tag;
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)cell->panel->node->parent->parent->data;
  mca_node_layout *layout = cell->panel->node->layout;

  switch (cell->type) {
  case MO_STEP_CONTEXT_PARAMETER: {
    _mc_mo_cpd_create_new_cell(cpd, MO_STEP_PROCESS_TITLE, layout->padding.left + layout->__bounds.width + 20.f,
                               layout->padding.top + 48.f, &cell->context_parameter.delegate);
    cell->continue_button->node->layout->visible = false;
  } break;
  default:
    printf("6728 TODO : %i\n", cell->type);
    return;
  }
}

void _mc_mo_cpd_cell_continue_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_mo_cpd_step_data *cell = button->tag;
  mc_create_process_dialog_data *cpd = (mc_create_process_dialog_data *)cell->panel->node->parent->parent->data;
  mca_node_layout *layout = cell->panel->node->layout;

  switch (cell->type) {
  case MO_STEP_PROCESS_TITLE: {
    _mc_mo_cpd_create_new_cell(cpd, MO_STEP_UNDESIGNATED, layout->padding.left,
                               layout->padding.top + layout->__bounds.height + 48.f, &cell->process_title.initial_step);
    cell->continue_button->node->layout->visible = false;
  } break;
  case MO_STEP_CONTEXT_PARAMETER: {
    _mc_mo_cpd_create_new_cell(cpd, MO_STEP_UNDESIGNATED, layout->padding.left,
                               layout->padding.top + layout->__bounds.height + 48.f, &cell->context_parameter.next);
    cell->continue_button->node->layout->visible = false;
  } break;
  default:
    printf("6728 TODO : %i\n", cell->type);
    return;
  }
}

int mc_mo_activate_create_process_dialog(mc_create_process_dialog_data *cpd, void *callback_state,
                                         void *callback_delegate)
{
  // Set Callback Info
  cpd->callback.state = callback_state;
  cpd->callback.result_delegate = callback_delegate;

  // Initial cell
  MCcall(_mc_mo_cpd_create_new_cell(cpd, MO_STEP_PROCESS_TITLE, 50.f, 50.f, NULL));

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
  cpd->cells.ary = (mc_mo_cpd_step_data *)malloc(sizeof(mc_mo_cpd_step_data) * cpd->cells.size);

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
  int a, b;
  char buf[64];
  mca_node_layout *layout;
  mcu_panel *panel;
  mcu_button *button;
  mcu_textbox *textbox;
  mcu_textblock *textblock;
  mcu_dropdown *dropdown;
  mc_mo_cpd_step_data *cell;

  // Panel
  MCcall(mcu_init_panel(module_node, &cpd->panel));

  layout = cpd->panel->node->layout;
  layout->padding = (mc_paddingf){60, 40, 60, 40};
  // TODO -- set up extents override so the dialog can adjust to the size of the message
  // layout->max_width = 520;
  // layout->max_height = 360;

  cpd->panel->background_color = (render_color){0.28f, 0.28f, 0.21f, 1.f};

  // Cells
  for (a = 0; a < cpd->cells.size; ++a) {
    cell = &cpd->cells.ary[a];

    cell->type = MO_STEP_NULL;

    MCcall(mcu_init_panel(cpd->panel->node, &cell->panel));
    panel = cell->panel;

    layout = panel->node->layout;
    layout->visible = false;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

    MCcall(mcu_init_dropdown(panel->node, &cell->dropdown));
    dropdown = cell->dropdown;

    dropdown->tag = cell;
    dropdown->selection = (void *)&_mc_mo_cpd_cell_dropdown_selection;

    layout = dropdown->node->layout;
    layout->padding = (mc_paddingf){4, 4, 4, 4};
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

    cell->textblocks = (mcu_textblock **)malloc(sizeof(mcu_textblock *) * MO_CPD_CELL_TEXTBLOCK_COUNT);
    for (b = 0; b < MO_CPD_CELL_TEXTBLOCK_COUNT; ++b) {
      MCcall(mcu_init_textblock(panel->node, &textblock));
      cell->textblocks[b] = textblock;

      textblock->background_color.a = 0.f;

      layout = textblock->node->layout;
      layout->padding = (mc_paddingf){4, 4, 4, 4};
      layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
      layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    }

    MCcall(mcu_init_textbox(panel->node, &cell->textbox));
    textbox = cell->textbox;

    layout = textbox->node->layout;
    layout->padding = (mc_paddingf){4, 4, 4, 4};
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

    MCcall(mcu_init_button(panel->node, &cell->delegate_button));
    button = cell->delegate_button;

    layout = button->node->layout;
    layout->preferred_width = 60;
    layout->preferred_height = 16;
    layout->padding = (mc_paddingf){4, 4, 4, 0};
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

    MCcall(set_mc_str(button->str, "obtain->"));
    button->tag = cell;
    button->left_click = (void *)&_mc_mo_cpd_cell_delegate_clicked;

    MCcall(mcu_init_button(panel->node, &cell->continue_button));
    button = cell->continue_button;

    layout = button->node->layout;
    layout->preferred_width = 16;
    layout->preferred_height = 16;
    layout->padding = (mc_paddingf){4, 4, 4, 0};
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

    MCcall(set_mc_str(button->str, "+"));
    button->tag = cell;
    button->left_click = (void *)&_mc_mo_cpd_cell_continue_clicked;
  }

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