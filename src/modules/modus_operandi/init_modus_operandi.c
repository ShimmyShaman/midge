/* init_modus_operandi */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/render_utilities/render_util.h"
#include "modules/ui_elements/ui_elements.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"

typedef enum mo_op_step_action_type {
  MO_OPPA_NULL = 0,
  MO_OPPA_OPEN_FILE_DIALOG,
  MO_OPPA_OPEN_FOLDER_DIALOG,
} mo_op_step_action_type;

typedef enum mo_op_step_context_arg {
  MO_OPPC_NULL = 0,
  MO_OPPC_ACTIVE_PROJECT_SRC_PATH,
} mo_op_step_context_arg;

typedef struct mo_operational_step {
  struct mo_operational_step *next;

  mo_op_step_action_type action;
  mo_op_step_context_arg context_arg;
  char *target_context_property;
} mo_operational_step;

struct modus_operandi_data;
typedef struct mo_operational_process {
  struct modus_operandi_data *mo_data;
  char *title;
  mo_operational_step *first;
} mo_operational_process;

typedef struct modus_operandi_data {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

  struct {
    unsigned int capacity, count;
    mo_operational_process **items;
  } all_ops;
  mo_operational_process *active_process;
  mo_operational_step *active_step;

  mcu_textbox *search_textbox;
  struct {
    unsigned int capacity, count;
    mcu_button **items;
  } options_buttons;

} modus_operandi_data;

void _mc_mo_render_mo_data_headless(render_thread_info *render_thread, mc_node *node)
{
  modus_operandi_data *mo_data = (modus_operandi_data *)node->data;

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
  irq->clear_color = COLOR_BLACKCURRANT;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = mo_data->render_target.width;   // TODO
  irq->image_height = mo_data->render_target.height; // TODO
  irq->data.target_image.image = mo_data->render_target.image;
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

void _mc_mo_render_mo_data_present(image_render_details *image_render_queue, mc_node *node)
{
  modus_operandi_data *mo_data = (modus_operandi_data *)node->data;

  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, mo_data->render_target.width,
                                         mo_data->render_target.height, mo_data->render_target.image);
}

void _mc_mo_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mc_mo_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_mo_activate_active_step(modus_operandi_data *mo_data)
{
  switch (mo_data->active_step->action) {
  case MO_OPPA_OPEN_FOLDER_DIALOG:
  default:
    MCerror(8142, "Unsupported action:%i", mo_data->active_step->action);
  }

  return 0;
}

void _mc_mo_operational_process_selected(mci_input_event *input_event, mcu_button *button)
{
  printf("input_event_type:%i\n", input_event->type);
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    mo_operational_process *mopp = (mo_operational_process *)button->tag;
    printf("mopp selected! : '%s'\n", mopp->title);

    // Begin the process
    modus_operandi_data *mo_data = mopp->mo_data;
    mo_data->active_process = mopp;
    mo_data->active_step = mopp->first;

    _mc_mo_activate_active_step(mo_data);
  }
}

int mc_mo_update_options_display(modus_operandi_data *mo_data)
{
  // Order according to search text match score
  // if(!mo_data->search_textbox->contents->len) {

  // }

  int a;
  mcu_button *button;
  mo_operational_process *mopp;
  for (a = 0; a < mo_data->all_ops.count && a < mo_data->options_buttons.count; ++a) {
    button = mo_data->options_buttons.items[a];
    mopp = mo_data->all_ops.items[a];

    MCcall(set_mc_str(button->str, mopp->title));

    button->tag = mopp;

    button->node->layout->visible = true;
    MCcall(mca_set_node_requires_rerender(button->node));
  }
  for (; a < mo_data->options_buttons.count; ++a) {
    button = mo_data->options_buttons.items[a];

    button->node->layout->visible = false;
  }

  MCcall(mca_set_node_requires_rerender(mo_data->node));

  return 0;
}

int mc_mo_load_resources(mc_node *module_node)
{
  // cube_template
  modus_operandi_data *mo_data = (modus_operandi_data *)malloc(sizeof(modus_operandi_data));
  module_node->data = mo_data;
  mo_data->node = module_node;

  mo_data->options_buttons.capacity = mo_data->options_buttons.count = 0U;
  mo_data->all_ops.capacity = mo_data->all_ops.count = 0U;

  mo_data->render_target.image = NULL;
  mo_data->render_target.width = module_node->layout->preferred_width;
  mo_data->render_target.height = module_node->layout->preferred_height;
  MCcall(mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
                                     MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image));

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!mo_data->render_target.image) {
    // puts("wait");
    usleep(100);
  }

  return 0;
}

int mc_mo_init_ui(mc_node *module_node)
{
  modus_operandi_data *mo_data = (modus_operandi_data *)module_node->data;

  MCcall(mcu_init_textbox(module_node, &mo_data->search_textbox));
  mo_data->search_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  mo_data->search_textbox->node->layout->padding = (mc_paddingf){6, 2, 6, 2};

  char buf[64];

  // unsigned int y = (unsigned int)(24 + 8 + 4);
  for (int a = 0; a < 12; ++a) {
    mcu_button *button;
    MCcall(mcu_init_button(module_node, &button));

    if (button->node->name) {
      free(button->node->name);
      button->node->name = NULL;
    }
    sprintf(buf, "mo-options-button-%i", a);
    button->node->name = strdup(buf);

    button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    button->node->layout->padding = (mc_paddingf){6, 24 + 8 + a * 27, 6, 0};
    button->node->layout->max_width = 0U;
    button->node->layout->visible = false;

    button->left_click = (void *)&_mc_mo_operational_process_selected;

    MCcall(set_mc_str(button->str, "button"));

    MCcall(append_to_collection((void ***)&mo_data->options_buttons.items, &mo_data->options_buttons.capacity,
                                &mo_data->options_buttons.count, button));
  }

  // // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  // while (!mo_data->render_target.image) {
  //   // puts("wait");
  //   usleep(100);
  // }

  return 0;
}

int mc_mo_load_operations(mc_node *module_node)
{
  modus_operandi_data *mo_data = (modus_operandi_data *)module_node->data;

  // Script an operational process for scripting operational processes
  {
    mo_operational_process *script_operational_process;
    mo_operational_step *step;

    script_operational_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
    script_operational_process->mo_data = mo_data;

    // Create
  }

  // Add Renderable System
  {
    mo_operational_process *add_renderable_system;
    mo_operational_step *step;

    add_renderable_system = (mo_operational_process *)malloc(sizeof(mo_operational_process));
    add_renderable_system->mo_data = mo_data;

    // Create the source files
    const char *const gen_source_folder_context_property = "gen-source-folder";
    {
      // Obtain the folder to create them in
      add_renderable_system->title = strdup("add-3d-renderable-system");
      step = add_renderable_system->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

      // -- Open Open-Path-Dialog - Seek a folder to store the source
      step->action = MO_OPPA_OPEN_FOLDER_DIALOG;
      // -- -- Folder to begin dialog with -- should be the src folder of the active project
      step->context_arg = MO_OPPC_ACTIVE_PROJECT_SRC_PATH;
      step->target_context_property = strdup(gen_source_folder_context_property);
      add_renderable_system->first->next = NULL;
    }
    {
      // Generate them
    }

    MCcall(append_to_collection((void ***)&mo_data->all_ops.items, &mo_data->all_ops.capacity, &mo_data->all_ops.count,
                                add_renderable_system));
  }

  MCcall(mc_mo_update_options_display(mo_data));

  return 0;
}

int init_modus_operandi(mc_node *app_root)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  mc_node *node;
  mca_init_mc_node(NODE_TYPE_ABSTRACT, "mod-op-root", &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 299;
  node->layout->preferred_height = 360;

  node->layout->padding.left = 1;
  node->layout->padding.bottom = 1;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_mo_render_mo_data_headless;
  node->layout->render_present = (void *)&_mc_mo_render_mo_data_present;
  node->layout->handle_input_event = (void *)&_mc_mo_handle_input;

  // TODO
  // node->layout->visible = false;
  // TODO

  MCcall(mc_mo_load_resources(node));

  MCcall(mc_mo_init_ui(node));

  MCcall(mc_mo_load_operations(node));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  // Panel
  // mcu_panel *panel;
  // mcu_init_panel(global_data->global_node, &panel);

  // panel->node->layout->padding = {300, 400, 800, 60};
  // panel->background_color = COLOR_GREEN;

  // mcu_set_element_update(panel->element);

  // Text Block
  // mcu_text_block *text_block;
  // mcu_init_text_block(global_data->global_node, &text_block);

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