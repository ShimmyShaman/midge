/* commander.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <dirent.h>
#include <unistd.h>

#include "core/midge_app.h"
#include "mc_error_handling.h"
#include "render/render_common.h"

// #include "modules/collections/hash_table.h"
// #include "modules/mc_io/mc_file.h"
// #include "modules/render_utilities/render_util.h"
// #include "modules/ui_elements/ui_elements.h"

// #include "modules/modus_operandi/create_process_dialog.h"
// #include "modules/modus_operandi/mo_context_viewer.h"
// #include "modules/modus_operandi/mo_serialization.h"
// #include "modules/modus_operandi/mo_types.h"
// #include "modules/modus_operandi/mo_util.h"
// #include "modules/modus_operandi/process_step_dialog.h"

typedef struct commander_data {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

//   mc_create_process_dialog_data *create_process_dialog;
//   mc_process_step_dialog_data *create_step_dialog;

//   mc_mo_process_stack process_stack;

//   mo_operational_process_list all_processes;

//   mcu_textbox *search_textbox;
//   struct {
//     unsigned int capacity, count;
//     mcu_button **items;
//   } options_buttons;

//   mc_node *context_viewer_node;

} commander_data;

void _mc_cmdr_render_mod_headless(render_thread_info *render_thread, mc_node *node)
{
  commander_data *data = (commander_data *)node->data;

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

  // Render the render target
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  image_render_details *irq;
  mcr_obtain_image_render_request(global_data->render_thread, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_GRAPE;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = data->render_target.width;   // TODO
  irq->image_height = data->render_target.height; // TODO
  irq->data.target_image.image = data->render_target.image;
  irq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  irq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_present(irq, child);
    }
  }

  mcr_submit_image_render_request(global_data->render_thread, irq);
}

void _mc_cmdr_render_present(image_render_details *image_render_queue, mc_node *node)
{
  commander_data *data = (commander_data *)node->data;

  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, data->render_target.width,
                                         data->render_target.height, data->render_target.image);
}

void _mc_cmdr_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mc_mo_handle_input\n");
  input_event->handled = true;
  // if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
  // }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

int mc_cmdr_init_ui(mc_node *module_node)
{
  commander_data *data = (commander_data *)module_node->data;

//   MCcall(mcu_init_textbox(module_node, &mod->search_textbox));
//   mod->search_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//   mod->search_textbox->node->layout->padding = (mc_paddingf){6, 2, 42, 2};

//   char buf[64];
//   mcu_button *button;

//   // Visible Options Buttons
//   for (int a = 0; a < 12; ++a) {
//     MCcall(mcu_alloc_button(module_node, &button));

//     if (button->node->name) {
//       free(button->node->name);
//       button->node->name = NULL;
//     }
//     sprintf(buf, "mo-options-button-%i", a);
//     button->node->name = strdup(buf);
//     button->text_align.horizontal = HORIZONTAL_ALIGNMENT_LEFT;

//     button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//     button->node->layout->padding = (mc_paddingf){6, 24 + 8 + a * 27, 6, 0};
//     button->node->layout->max_width = 0U;
//     button->node->layout->visible = false;

//     button->left_click = (void *)&_mc_mo_operational_process_selected;

//     MCcall(mc_set_str(&button->str, "button"));

//     MCcall(append_to_collection((void ***)&mod->options_buttons.items, &mod->options_buttons.capacity,
//                                 &mod->options_buttons.count, button));
//   }

//   // Add process button
//   MCcall(mcu_alloc_button(module_node, &button));
//   if (button->node->name) {
//     free(button->node->name);
//     button->node->name = NULL;
//   }
//   button->node->name = strdup("mo-create-process-button");

//   button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
//   button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
//   button->node->layout->padding = (mc_paddingf){6, 2, 6, 2};
//   button->node->layout->max_width = 16U;
//   button->node->layout->max_height = 16U;
//   button->tag = mod;

//   button->left_click = (void *)&_mc_mo_create_process_clicked;

//   MCcall(mc_set_str(&button->str, "+"));

//   // Show Context Viewer button
//   MCcall(mcu_alloc_button(module_node, &button));
//   if (button->node->name) {
//     free(button->node->name);
//     button->node->name = NULL;
//   }
//   button->node->name = strdup("mo-show-context-viewer-button");

//   button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
//   button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
//   button->node->layout->padding = (mc_paddingf){6, 2, 6, 2};
//   button->node->layout->max_width = 18U;
//   button->node->layout->max_height = 18U;
//   button->tag = mod;

//   button->left_click = (void *)&_mc_mo_toggle_context_viewer_clicked;

//   MCcall(mc_set_str(&button->str, "oo"));

  return 0;
}

int mc_cmdr_load_resources(mc_node *module_node)
{
  int a;

  // Initialize
  commander_data *data = (commander_data *)malloc(sizeof(commander_data));
  module_node->data = data;
  data->node = module_node;

//   mod->options_buttons.capacity = mod->options_buttons.count = 0U;
//   mod->all_processes.capacity = mod->all_processes.count = 0U;

//   mod->process_stack.index = -1;
//   mod->process_stack.state_arg = (void *)mod;
//   mod->process_stack.all_processes = &mod->all_processes;
//   MCcall(init_hash_table(64, &mod->process_stack.global_context));
//   MCcall(init_hash_table(4, &mod->process_stack.project_contexts));
//   for (a = 0; a < MO_OP_PROCESS_STACK_SIZE; ++a) {
//     MCcall(init_hash_table(8, &mod->process_stack.context_maps[a]));
//     mod->process_stack.processes[a] = NULL;
//     mod->process_stack.steps[a] = NULL;
//   }

  data->render_target.image = NULL;
  data->render_target.width = module_node->layout->preferred_width;
  data->render_target.height = module_node->layout->preferred_height;
  MCcall(mcr_create_texture_resource(data->render_target.width, data->render_target.height,
                                     MVK_IMAGE_USAGE_RENDER_TARGET_2D, &data->render_target.image));

  return 0;
}

int init_commander_system(mc_node *app_root) {

  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  mc_node *node;
  mca_init_mc_node(NODE_TYPE_DOESNT_MATTER, "commander-root", &node);
  mca_init_node_layout(&node->layout);

  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 349;
  node->layout->preferred_height = 140;

  node->layout->padding.left = 2;
  node->layout->padding.bottom = 2;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_cmdr_render_mod_headless;
  node->layout->render_present = (void *)&_mc_cmdr_render_present;
  node->layout->handle_input_event = (void *)&_mc_cmdr_handle_input;

  // TODO
  // node->layout->visible = false;
  // TODO

  MCcall(mc_cmdr_load_resources(node));
  commander_data *data = (commander_data *)node->data;

//   MCcall(mc_cmdr_init_ui(node));

//   MCcall(_mc_mo_load_operations(node));

//   // Create Process Step Dialog
//   MCcall(mc_mo_init_create_process_dialog(app_root, &mod->create_process_dialog));
//   MCcall(mc_mocsd_init_process_step_dialog(app_root, &mod->create_step_dialog));

  // Event Registers
//   MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_STRUCTURE_CREATION, &_mc_mo_project_created, node->data));
//   MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_LOADED, &_mc_mo_project_loaded, node->data));
  // TODO -- register for project closed/shutdown


  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!data->render_target.image) {
    // puts("wait");
    usleep(100);
  }
  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  return 0;
}