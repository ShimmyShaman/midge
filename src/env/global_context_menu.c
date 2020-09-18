
#include "env/environment_definitions.h"
#include "ui/ui_definitions.h"

void mca_handle_global_context_menu_option_selected(const char *selected_option)
{
  if (!strcmp(selected_option, "Add Button")) {
    global_root_data *global_data;
    obtain_midge_global_root(&global_data);

    switch (global_data->ui_state->global_context_menu_context_node->type) {
    case NODE_TYPE_VISUAL_PROJECT: {
      mui_button *button;
      mui_init_button(global_data->ui_state->global_context_menu_context_node, &button);

      set_c_str(button->str, "button");

    } break;
    default:
      MCerror(9815, "TODO");
    }
    return;
  }
  // printf("unhandled menu option selected:'%s'", selected_option);
}

void mca_init_global_context_menu()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_context_menu *context_menu;
  mui_init_context_menu(global_data->global_node, &context_menu);

  mca_modify_z_layer_index(context_menu->element->visual_node, 10U);

  // Set to global
  global_data->ui_state->global_context_menu = context_menu->element->visual_node;
  global_data->ui_state->global_context_menu_context_node = NULL;
  context_menu->element->visual_node->visible = false;

  context_menu->element->layout->padding = {150, 200, 0, 0};
  context_menu->element->layout->preferred_width = 120.f;
  context_menu->element->layout->preferred_height = 90.f;
  context_menu->element->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  context_menu->element->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  context_menu->background_color = COLOR_DARK_SLATE_GRAY;
  context_menu->option_selected = (void *)&mca_handle_global_context_menu_option_selected;
}

void mca_render_global_context_menu(image_render_queue *render_queue, mc_node *node) {}

void mca_gcm_handler(mc_node event_node) { printf("!!!!It Registered!!!!!\n"); }

void mca_activate_global_context_menu(mc_node *context_node, int screen_x, int screen_y)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Show
  // printf("mca_activate_global_context_menu--node %p\n", node);
  global_data->ui_state->global_context_menu->visible = true;
  global_data->ui_state->global_context_menu_context_node = context_node;

  // Set New Layout
  mui_ui_element *gcm_element = (mui_ui_element *)global_data->ui_state->global_context_menu->data;
  gcm_element->layout->padding = {(float)screen_x, (float)screen_y, 0, 0};

  mui_context_menu_clear_options(gcm_element);

  // Alter available options depending on the type of context_node activated on
  switch (context_node->type) {
  case NODE_TYPE_VISUAL_PROJECT: {
    mui_context_menu_add_option(gcm_element, "Add Button");
  } break;
  case NODE_TYPE_UI: {
    mui_ui_element *element = (mui_ui_element *)context_node->data;
    switch (element->type) {
    case UI_ELEMENT_BUTTON: {

    } break;
    default:
      // Respond with the default options
      mui_context_menu_add_option(gcm_element, "Add Button");
      // TODO make this a (none) disabled button
      break;
    }
  } break;
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