
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
  context_menu->element->visual_node->visible = false;

  context_menu->element->layout->padding = {150, 200, 0, 0};
  context_menu->element->layout->preferred_width = 120.f;
  context_menu->element->layout->preferred_height = 90.f;
  context_menu->element->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  context_menu->element->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  context_menu->background_color = COLOR_DARK_SLATE_GRAY;
}

void mca_render_global_context_menu(image_render_queue *render_queue, mc_node *node) {}

void mca_gcm_handler(mc_node event_node) { printf("!!!!It Registered!!!!!\n"); }

void mca_activate_global_context_menu(mc_node *node, int screen_x, int screen_y)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Show
  // printf("mca_activate_global_context_menu--node %p\n", node);
  global_data->ui_state->global_context_menu->visible = true;

  // Set New Layout
  mui_ui_element *gcm_element = (mui_ui_element *)global_data->ui_state->global_context_menu->data;
  gcm_element->layout->padding = {(float)screen_x, (float)screen_y, 0, 0};

  mui_context_menu_clear_options(gcm_element);

  // Alter available options depending on the type of node activated on
  switch (node->type) {
  default:
    // Respond with the default options
    mui_context_menu_add_option(gcm_element, "Add Button");
    break;
  }

  mca_set_node_requires_layout_update(gcm_element->visual_node);
}

void mca_hide_global_context_menu()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->ui_state->global_context_menu->visible = true;

  mca_set_node_requires_layout_update(global_data->ui_state->global_context_menu);
}