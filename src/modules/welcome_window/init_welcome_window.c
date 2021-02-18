/* init_modus_operandi */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "core/app_modules.h"
#include "core/midge_app.h"
#include "mc_error_handling.h"
#include "render/render_common.h"

#include "modules/mc_io/mc_projects.h"
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

void mc_mww_render_headless(render_thread_info *render_thread, mc_node *node)
{
  welcome_window_data *wwwata = (welcome_window_data *)node->data;

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

void mc_mww_render_present(image_render_details *image_render_queue, mc_node *node)
{
  welcome_window_data *wwwata = (welcome_window_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, wwwata->background_color);

  // Children
  mca_render_node_list_present(image_render_queue, node->children);
}

void mc_mww_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mco_handle_input\n");
  input_event->handled = true;
  // if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
  // }
}

void _mc_mww_on_new_project(mci_input_event *input_event, mcu_button *button)
{
  welcome_window_data *ww = (welcome_window_data *)button->node->parent->data;

  // Make the input textbox visible
  ww->input_textbox->node->layout->visible = true;
  input_event->focus_successor = ww->input_textbox->node;
  mca_set_node_requires_rerender(ww->input_textbox->node);
}

void _mc_mww_textbox_submit(mci_input_event *input_event, mcu_textbox *textbox)
{
  if (!textbox->contents->len) {
    puts("5106 - TODO - error handling");
    return;
  }

  welcome_window_data *ww = (welcome_window_data *)textbox->node->parent->data;
  ww->input_textbox->node->layout->visible = false;

  char buf[256];
  char *cdr = getcwd(buf, 256); // TODO sizeof pointer instead of type in mc
  if (!cdr) {
    puts("5415 - TODO - error handling");
    return;
  }

  // The current working directory... maybe should be more fixed or set on start or application directory??? TODO
  // This is the directory you'd ask the user to provide if you had a more elaborate system
  char c = buf[strlen(buf) - 1];
  if (c != '\\' && c != '/')
    strcat(buf, "/");
  strcat(buf, "projects");

  // Clear to create the project
  // -- create the project folder
  if (mcf_create_project_file_structure(buf, textbox->contents->text)) {
    MCVerror(5928, "TODO");
  }

  // Load the created project
  // TODO -- maybe include create_project in the async call too .. maybe it takes too long?? not a priority
  mca_load_project_async(buf, textbox->contents->text);

  // Close the welcome window
  ww->node->layout->visible = false;
}

int _mc_mww_on_inital_load_complete(void *state, void *event_arg)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // printf("_mc_mww_on_inital_load_complete:%i\n", app_info->projects.count);
  if (!app_info->projects.count) {
    // No projects on initial load, nows the time to shine baby
    mc_node *node = (mc_node *)state;
    node->layout->visible = true;
    MCcall(mca_set_node_requires_rerender(node));

    // puts("time to shine baby");
  }

  return 0;
}

int mc_mww_init_data(mc_node *module_node)
{
  // cube_template
  welcome_window_data *ww = (welcome_window_data *)malloc(sizeof(welcome_window_data));
  module_node->data = ww;
  ww->node = module_node;

  ww->background_color = COLOR_CORNFLOWER_BLUE;

  MCcall(mcu_alloc_button(module_node, &ww->new_project_button));
  ww->new_project_button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  ww->new_project_button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
  mc_set_str(&ww->new_project_button->str, "New Project");
  ww->new_project_button->node->layout->preferred_width = 120;
  ww->new_project_button->node->layout->preferred_height = 28;
  ww->new_project_button->left_click = &_mc_mww_on_new_project;

  MCcall(mcu_init_textbox(module_node, &ww->input_textbox));
  ww->input_textbox->node->layout->visible = false;
  ww->input_textbox->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  ww->input_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;
  mc_set_str(&ww->new_project_button->str, "New Project");
  ww->input_textbox->node->layout->preferred_width = 160;
  ww->input_textbox->node->layout->preferred_height = 32;
  ww->input_textbox->submit = &_mc_mww_textbox_submit;

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
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);
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

  node->layout->visible = false;

  // node->layout->padding.left = 120;
  // node->layout->padding.bottom = 80;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&mc_mww_render_headless;
  node->layout->render_present = (void *)&mc_mww_render_present;
  node->layout->handle_input_event = (void *)&mc_mww_handle_input;

  mc_mww_init_data(node);

  MCcall(
      mca_register_event_handler(MC_APP_EVENT_INITIAL_MODULES_PROJECTS_LOADED, _mc_mww_on_inital_load_complete, node));

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

  // mc_set_str(text_block->str, "");
  // for (int a = 32; a < 128; ++a) {
  //   char buf[2];
  //   buf[0] = (char)a;
  //   buf[1] = '\0';
  //   mc_append_to_str(text_block->str, buf);
  // }
  // text_block->font_color = COLOR_LIGHT_YELLOW;

  return 0;
}