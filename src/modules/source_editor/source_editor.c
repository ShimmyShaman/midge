/* source_editor.c */

#include <stdio.h>
#include <stdlib.h>

#include "control/mc_controller.h"
#include "core/midge_app.h"
#include "env/environment_definitions.h"

#include "modules/source_editor/source_editor.h"

void _mc_se_render_headless(mc_node *node)
{
  // Data
  mc_se_source_editor *se = (mc_se_source_editor *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }
}

void _mc_se_render_present(image_render_details *image_render_queue, mc_node *node)
{
  // Data
  mc_se_source_editor *se = (mc_se_source_editor *)node->data;

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, se->background_color);
}

void _mc_se_handle_input(mc_node *node, mci_input_event *input_event)
{
  // Data
  mc_se_source_editor *se = (mc_se_source_editor *)node->data;

  // printf("_mco_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_se_handle_source_file_open_request(void *handler_state, void *event_args)
{
  // Data
  mc_se_source_editor *se = (mc_se_source_editor *)handler_state;
  const char *filepath = (const char *)event_args;

  printf("SE_OPEN:'%s'\n", filepath);

  mca_focus_node(se->node);
  return 0;
}

int _mc_se_load_resources(mc_node *node)
{
  // Data
  mc_se_source_editor *se = (mc_se_source_editor *)node->data;

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  // while (!mh_data->cube.render_data.input_buffers[1]) {
  //   // puts("wait");
  //   usleep(100);
  // }
  return 0;
}

int _mc_se_init_data(mc_node *node)
{
  mc_se_source_editor *se = (mc_se_source_editor *)malloc(sizeof(mc_se_source_editor));
  node->data = (void *)se;
  se->node = node;

  se->background_color = COLOR_LIGHT_YELLOW;

  return 0;
}

int mc_se_init_source_editor()
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "source-editor", &node));
  MCcall(mca_init_node_layout(&node->layout));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  // node->layout->preferred_width = 400;
  // node->layout->preferred_height = 500;

  node->layout->padding = (mc_paddingf){280, 80, 80, 80};
  // node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  // node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_se_render_headless;
  node->layout->render_present = (void *)&_mc_se_render_present;
  node->layout->handle_input_event = (void *)&_mc_se_handle_input;

  // TODO
  //   node->layout->visible = false;
  //
  // Source Editor Data
  MCcall(_mc_se_init_data(node));

  // Event Registers
  MCcall(mca_register_event_handler(MC_APP_EVENT_SOURCE_FILE_OPEN_REQUESTED, &_mc_se_handle_source_file_open_request,
                                    node->data));

  // Graphical resources
  MCcall(_mc_se_load_resources(node));

  // Attach to midge hierarchy & return
  MCcall(mca_attach_node_to_hierarchy(app_info->global_node, node));
  return 0;
}