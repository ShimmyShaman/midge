/* init_modus_operandi */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "core/app_modules.h"
#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/mc_io/mc_io.h"
#include "modules/ui_elements/ui_elements.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"

typedef struct mc_file_dialog_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;

  //   mcu_button *new_project_button;
  //   mcu_textbox *input_textbox;
  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

} mc_file_dialog_data;

void mc_fd_render_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)node->data;

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

void mc_fd_render_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, fd->shade_color);

  //   mcr_issue_render_command_colored_quad(
  //       image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       fd->background_color);

  // Children
  mca_render_typical_nodes_children_present(image_render_queue, node->children);
}

void mc_fd_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mco_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

void _mc_fd_confirm_clicked(mci_input_event *input_event, mcu_button *button)
{
  // TODO
}

int mc_fd_init_data(mc_node *module_node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)malloc(sizeof(mc_file_dialog_data));
  module_node->data = fd;
  fd->node = module_node;

  fd->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  // mo_data->render_target.image = NULL;
  // mo_data->render_target.width = module_node->layout->preferred_width;
  // mo_data->render_target.height = module_node->layout->preferred_height;
  // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  //   while (!fd->render_target.image) {
  //     // puts("wait");
  //     usleep(100);
  //   }

  return 0;
}

int mc_fd_init_ui(mc_node *module_node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)module_node->data;

  mca_node_layout *layout;

  // Panel
  MCcall(mcu_init_panel(module_node, &fd->panel));

  layout = fd->panel->node->layout;
  layout->max_width = 320;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  layout->max_height = 540;

  fd->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  // Open Button
  mcu_button *button;
  MCcall(mcu_init_button(fd->panel, &button));

  layout = button->node->layout;
  layout->preferred_width = 160;
  layout->preferred_height = 26;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(set_mc_str(button->str, "Open"));
  button->tag = fd;
  button->left_click = (void *)&_mc_fd_confirm_clicked;

  return 0;
}

int mc_fd_init_file_dialog(mc_node *app_root)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  // TODO -- get rid of node type

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "file-dialog", &node));
  MCcall(mca_init_node_layout(&node->layout));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  node->layout->z_layer_index = 9;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&mc_fd_render_headless;
  node->layout->render_present = (void *)&mc_fd_render_present;
  node->layout->handle_input_event = (void *)&mc_fd_handle_input;

  MCcall(mc_fd_init_data(node));
  MCcall(mc_fd_init_ui(node));

  //   MCcall(
  //       mca_register_event_handler(MC_APP_EVENT_INITIAL_MODULES_PROJECTS_LOADED, _mc_fd_on_inital_load_complete,
  //       node));

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