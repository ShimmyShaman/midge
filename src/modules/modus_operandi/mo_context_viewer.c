/* mo_context_viewer.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/midge_app.h"
#include "env/environment_definitions.h"

#include "modules/modus_operandi/mo_context_viewer.h"
#include "modules/render_utilities/render_util.h"
#include "modules/ui_elements/ui_elements.h"

typedef struct mc_mo_context_viewer_data {
  mc_node *node;

  mc_mo_process_stack *process_stack;

  render_color background_color;
} mc_mo_context_viewer_data;

void _mc_mo_cv_render_headless(render_thread_info *render_thread, mc_node *node)
{
  // mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)node->data;

  // Children
  mca_render_node_list_headless(render_thread, node->children);

  // mcr_submit_image_render_request(app_info->render_thread, irq);
}

void _mc_mo_cv_render_present(image_render_details *irq, mc_node *node)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)node->data;

  // printf("_mc_mo_cv_render_present : %s\n", node->layout->visible ? "visible" : "not-visible");

  // mcr_issue_render_command_textured_quad(irq, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      irq, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, cv->background_color);

  mcr_render_border(irq, node, 1, COLOR_GHOST_WHITE);

  //   mcr_issue_render_command_colored_quad(
  //       irq, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       cv->background_color);

  // Children
  mca_render_node_list_present(irq, node->children);

  // mc_mo_cv_step_data *cell;
  // for (cell = cv->cells.ary; cell < &cv->cells.ary[cv->cells.size]; ++cell) {

  //   if (!cell->panel->node->layout->visible)
  //     break;

  //   printf("cell %i %i: %.2f %.2f %.2f %.2f\n", cell - cv->cells.ary, cell->type,
  //          cell->panel->node->layout->__bounds.x, cell->panel->node->layout->__bounds.y,
  //          cell->panel->node->layout->__bounds.width, cell->panel->node->layout->__bounds.height);
  // }
}

void _mc_mo_cv_handle_input(mc_node *node, mci_input_event *input_event)
{
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);

    input_event->handled = true;
  }
}

void _mc_mo_cv_cancel_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)button->tag;

  cv->node->layout->visible = false;
  mca_set_node_requires_rerender(cv->node);
}

////////////////////////////////////////////////////////////
////////////////      Initialization      //////////////////
////////////////////////////////////////////////////////////

int _mc_mo_cv_init_data(mc_node *module_node, mc_mo_process_stack *process_stack)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)malloc(sizeof(mc_mo_context_viewer_data));
  module_node->data = cv;
  cv->node = module_node;

  cv->process_stack = process_stack;

  // cv->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  // cv->callback.state = NULL;
  // cv->callback.result_delegate = NULL;

  // cv->cells.size = 32;
  // cv->cells.ary = (mc_mo_cv_step_data *)malloc(sizeof(mc_mo_cv_step_data) * cv->cells.size);

  // // mo_data->render_target.image = NULL;
  // // mo_data->render_target.width = module_node->layout->preferred_width;
  // // mo_data->render_target.height = module_node->layout->preferred_height;
  // // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  // //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  // //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  // //   while (!cv->render_target.image) {
  // //     // puts("wait");
  // //     usleep(100);
  // //   }

  return 0;
}

int _mc_mo_cv_init_ui(mc_node *module_node)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)module_node->data;

  cv->background_color = COLOR_MIDNIGHT_EXPRESS;

  // Locals
  int a, b;
  // char buf[64];
  mca_node_layout *layout;
  // mcu_panel *panel;
  mcu_button *button;
  // mcu_textbox *textbox;
  // mcu_textblock *textblock;
  // mcu_dropdown *dropdown;
  // mc_mo_cv_step_data *cell;

  // // Panel
  // MCcall(mcu_init_panel(module_node, &cv->panel));

  // layout = cv->panel->node->layout;
  // layout->padding = (mc_paddingf){60, 40, 60, 40};
  // // TODO -- set up extents override so the dialog can adjust to the size of the message
  // // layout->max_width = 520;
  // // layout->max_height = 360;

  // cv->panel->background_color = (render_color){0.28f, 0.28f, 0.21f, 1.f};

  // // Cells
  // for (a = 0; a < cv->cells.size; ++a) {
  //   cell = &cv->cells.ary[a];

  //   cell->type = MO_STEP_NULL;

  //   MCcall(mcu_init_panel(cv->panel->node, &cell->panel));
  //   panel = cell->panel;

  //   layout = panel->node->layout;
  //   layout->visible = false;
  //   layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  //   layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  //   MCcall(mcu_init_dropdown(panel->node, &cell->dropdown));
  //   dropdown = cell->dropdown;

  //   dropdown->tag = cell;
  //   dropdown->selection = (void *)&_mc_mo_cv_cell_dropdown_selection;

  //   layout = dropdown->node->layout;
  //   layout->padding = (mc_paddingf){4, 4, 4, 4};
  //   layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  //   layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  //   cell->textblocks = (mcu_textblock **)malloc(sizeof(mcu_textblock *) * MO_cv_CELL_TEXTBLOCK_COUNT);
  //   for (b = 0; b < MO_cv_CELL_TEXTBLOCK_COUNT; ++b) {
  //     MCcall(mcu_init_textblock(panel->node, &textblock));
  //     cell->textblocks[b] = textblock;

  //     textblock->background_color.a = 0.f;

  //     layout = textblock->node->layout;
  //     layout->padding = (mc_paddingf){4, 4, 4, 4};
  //     layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  //     layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  //   }

  //   MCcall(mcu_init_textbox(panel->node, &cell->textbox));
  //   textbox = cell->textbox;

  //   layout = textbox->node->layout;
  //   layout->padding = (mc_paddingf){4, 4, 4, 4};
  //   layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  //   layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  //   MCcall(mcu_init_button(panel->node, &cell->delegate_button));
  //   button = cell->delegate_button;

  //   layout = button->node->layout;
  //   layout->preferred_width = 60;
  //   layout->preferred_height = 16;
  //   layout->padding = (mc_paddingf){4, 4, 4, 0};
  //   layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  //   layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  //   MCcall(set_mc_str(button->str, "obtain->"));
  //   button->tag = cell;
  //   button->left_click = (void *)&_mc_mo_cv_cell_delegate_clicked;

  //   MCcall(mcu_init_button(panel->node, &cell->continue_button));
  //   button = cell->continue_button;

  //   layout = button->node->layout;
  //   layout->preferred_width = 16;
  //   layout->preferred_height = 16;
  //   layout->padding = (mc_paddingf){4, 4, 4, 0};
  //   layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  //   layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  //   MCcall(set_mc_str(button->str, "+"));
  //   button->tag = cell;
  //   button->left_click = (void *)&_mc_mo_cv_cell_continue_clicked;
  // }

  // Exit Button
  MCcall(mcu_init_button(cv->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 16;
  layout->preferred_height = 16;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(set_mc_str(button->str, "X"));
  button->tag = cv;
  button->left_click = (void *)&_mc_mo_cv_cancel_clicked;

  return 0;
}

int mc_mo_toggle_context_viewer_visibility(mc_node *node)
{
  // mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)module_node->data;

  node->layout->visible = !node->layout->visible;

  return 0;
}

// TODO remove @parent
int init_mo_context_viewer(mc_mo_process_stack *process_stack, mc_node **p_context_viewer)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // TODO -- get rid of node type
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "context-viewer", &node));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  MCcall(mca_init_node_layout(&node->layout));
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
  node->layout->max_width = 320;
  node->layout->max_height = 480;

  node->layout->visible = false;
  node->layout->z_layer_index = 5U;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_mo_cv_render_headless;
  node->layout->render_present = (void *)&_mc_mo_cv_render_present;
  node->layout->handle_input_event = (void *)&_mc_mo_cv_handle_input;

  MCcall(_mc_mo_cv_init_data(node, process_stack));
  MCcall(_mc_mo_cv_init_ui(node));

  // MCcall(mca_register_event_handler(MC_APP_EVENT_create_process_dialog_REQUESTED,
  // _mc_mo_create_process_dialog_requested,
  //                                   node->data));

  MCcall(mca_attach_node_to_hierarchy(app_info->global_node, node));
  *p_context_viewer = node;

  return 0;
}