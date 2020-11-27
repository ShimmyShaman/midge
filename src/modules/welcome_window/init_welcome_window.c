/* init_modus_operandi */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/ui_elements/ui_elements.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"

typedef struct welcome_window_data {
  mc_node *node;

  render_color background_color;

  mcu_button *new_project_button;
  mcu_textbox *input_textbox;
  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

} welcome_window_data;

void mww_render_headless(mc_node *node)
{
  welcome_window_data *wwdata = (welcome_window_data *)node->data;

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

  // // Render the render target
  // midge_app_info *global_data;
  // mc_obtain_midge_app_info(&global_data);

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

  // mcr_submit_image_render_request(global_data->render_thread, irq);
}

void mww_render_present(image_render_details *image_render_queue, mc_node *node)
{
  welcome_window_data *wwdata = (welcome_window_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, wwdata->background_color);

  // Children
  mca_render_typical_nodes_children_present(image_render_queue, node->children);
}

void mww_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mco_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

void _mww_on_new_project(mcu_button *button, mc_point click_location) {}

void _mww_textbox_submit(mcu_textbox *textbox, mc_point click_location) { puts(textbox->contents->text); }

int mww_init_data(mc_node *module_node)
{
  // cube_template
  welcome_window_data *wd = (welcome_window_data *)malloc(sizeof(welcome_window_data));
  module_node->data = wd;
  wd->node = module_node;

  wd->background_color = COLOR_CORNFLOWER_BLUE;

  MCcall(mcu_init_button(module_node, &wd->new_project_button));
  wd->new_project_button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  wd->new_project_button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
  set_mc_str(wd->new_project_button->str, "New Project");
  wd->new_project_button->node->layout->preferred_width = 120;
  wd->new_project_button->node->layout->preferred_height = 28;
  wd->new_project_button->left_click = &_mww_on_new_project;

  MCcall(mcu_init_textbox(module_node, &wd->input_textbox));
  wd->input_textbox->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  wd->input_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
  set_mc_str(wd->new_project_button->str, "New Project");
  wd->input_textbox->node->layout->preferred_width = 160;
  wd->input_textbox->node->layout->preferred_height = 32;
  wd->input_textbox->submit = &_mww_textbox_submit;

  // mo_data->render_target.image = NULL;
  // mo_data->render_target.width = module_node->layout->preferred_width;
  // mo_data->render_target.height = module_node->layout->preferred_height;
  // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  // while (!mo_data->render_target.image) {
  //   // puts("wait");
  //   usleep(100);
  // }

  return 0;
}

int init_welcome_window(mc_node *app_root)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  // TODO -- get rid of node type

  mc_node *node;
  mca_init_mc_node(NODE_TYPE_ABSTRACT, "welcome_window-root", &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 220;
  node->layout->preferred_height = 140;

  // node->layout->padding.left = 120;
  // node->layout->padding.bottom = 80;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&mww_render_headless;
  node->layout->render_present = (void *)&mww_render_present;
  node->layout->handle_input_event = (void *)&mww_handle_input;

  mww_init_data(node);

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