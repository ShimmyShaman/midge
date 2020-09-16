
#include "env/environment_definitions.h"
#include "ui/ui_definitions.h"

void mca_init_global_context_menu()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_context_menu *context_menu;
  mui_init_context_menu(global_data->global_node, &context_menu);

  // Set to global
  global_data->ui_state->global_context_menu = context_menu->element->visual_node;

  context_menu->element->bounds = {150, 200, 140, 220};
  context_menu->background_color = COLOR_DARK_SLATE_GRAY;
  context_menu->element->visible = false;
}

void mca_render_global_context_menu(image_render_queue *render_queue, mc_node *node) {}

void mca_gcm_handler(mc_node event_node) { printf("!!!!It Registered!!!!!\n"); }

void mca_activate_global_context_menu(mc_node *node, int screen_x, int screen_y)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_ui_element *gcm_element = (mui_ui_element *)global_data->ui_state->global_context_menu->data;

  gcm_element->bounds = {(float)screen_x, (float)screen_y, gcm_element->bounds.width, gcm_element->bounds.height};
  gcm_element->visible = true;

  mui_context_menu_clear_options(gcm_element);

  // Alter available options depending on the type of node activated on
  switch (node->type) {
  default:
    // Respond with the default options
    mui_context_menu_add_option(gcm_element, "Add Button");
    break;
  }

  mca_set_node_requires_update(gcm_element->visual_node);
}

void mca_hide_global_context_menu()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_ui_element *gcm_element = (mui_ui_element *)global_data->ui_state->global_context_menu->data;
  gcm_element->visible = true;

  mca_set_node_requires_update(gcm_element->visual_node);
}