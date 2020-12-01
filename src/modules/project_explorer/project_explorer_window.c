/* project_explorer_window.c */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "core/midge_app.h"
#include "env/environment_definitions.h"

#include "modules/project_explorer/project_explorer_window.h"

typedef struct project_explorer_data {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

} project_explorer_data;

void _mcm_pjxp_render_headless(mc_node *node)
{
  // Toggle
  node->layout->__requires_rerender = false;

  // Data
  project_explorer_data *data = (project_explorer_data *)node->data;

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

void _mcm_pjxp_render_present(image_render_details *image_render_queue, mc_node *node)
{
  // Toggle
  node->layout->__requires_rerender = false;

  // Data
  project_explorer_data *data = (project_explorer_data *)node->data;

  // Render Image
  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, data->render_target.width,
                                         data->render_target.height, data->render_target.image);
}

void _mcm_pjxp_handle_input(mc_node *node, mci_input_event *input_event)
{
  // Data
  project_explorer_data *data = (project_explorer_data *)node->data;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    // printf("obb\n");
    // if (input_event->button_code == MOUSE_BUTTON_LEFT) {

    //   // printf("left_click:offset=%i %.3f  line_index:%i\n", input_event->input_state->mouse.y,
    //   // node->layout->__bounds.y,
    //   //        (int)((input_event->input_state->mouse.y - node->layout->__bounds.y - fedit->lines.padding.top) /
    //   //              fedit->lines.vertical_stride));
    //   int line_index = -1;
    //   int click_relative_y =
    //       input_event->input_state->mouse.y - (int)(node->layout->__bounds.y + fedit->lines.padding.top);
    //   if (click_relative_y >= 0) {
    //     line_index = (int)((float)click_relative_y / fedit->lines.vertical_stride);
    //   }
    //   if (line_index >= 0) {
    //     // Find the column index
    //     int click_relative_x =
    //         input_event->input_state->mouse.x -
    //         (int)(node->layout->__bounds.x + fedit->lines.padding.left - fedit->font_horizontal_stride * 0.5f);
    //     if (click_relative_x < 0 && click_relative_x > -3)
    //       click_relative_x = 0;
    //     if (click_relative_x >= 0) {

    //       mce_set_function_editor_cursor_position(fedit, line_index,
    //                                               (int)((float)click_relative_x / fedit->font_horizontal_stride));
    //     }
    //   }
    // }

    // mca_focus_node(node);
    input_event->handled = true;
  }
}

int _mcm_pjxp_project_loaded(void *handler_state, void *event_args)
{
  // Data
  project_explorer_data *data = (project_explorer_data *)handler_state;

  data->node->layout->visible = true;

  MCcall(mca_set_node_requires_rerender(data->node));

  return 0;
}

int _mcm_pjxp_init_data(mc_node *root)
{
  project_explorer_data *data = (project_explorer_data *)malloc(sizeof(project_explorer_data));
  data->node = root;
  root->data = data;

  data->render_target.width = root->layout->preferred_width;
  data->render_target.height = root->layout->preferred_height;

  mcr_create_texture_resource(data->render_target.width, data->render_target.height, MVK_IMAGE_USAGE_RENDER_TARGET_3D,
                              &data->render_target.image);

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!data->render_target.image) {
    // puts("wait");
    usleep(100);
  }
  return 0;
}

int mcm_init_project_explorer(mc_node *app_root)
{
  //   midge_app_info *global_data;
  //   mc_obtain_midge_app_info(&global_data);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  // TODO -- get rid of node type

  mc_node *node;
  mca_init_mc_node(NODE_TYPE_ABSTRACT, "project-explorer-root", &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 180;
  node->layout->preferred_height = 420;

  node->layout->visible = false;

  node->layout->padding.left = 4;
  node->layout->padding.top = 4;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mcm_pjxp_render_headless;
  node->layout->render_present = (void *)&_mcm_pjxp_render_present;
  node->layout->handle_input_event = (void *)&_mcm_pjxp_handle_input;

  _mcm_pjxp_init_data(node);

  MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_LOADED, (void *)_mcm_pjxp_project_loaded, (void *)node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));
  return 0;
}