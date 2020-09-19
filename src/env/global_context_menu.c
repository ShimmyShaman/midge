
#include "env/environment_definitions.h"
#include "ui/ui_definitions.h"

void mca_handle_global_context_menu_option_selected(const char *selected_option)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
  mc_node *context_node = global_data->ui_state->global_context_menu_context_node;

  // TODO register control options and handling seperately in their modules

  if (!strcmp(selected_option, "Cancel")) {
    // Do nothing
    return;
  }
  if (!strcmp(selected_option, "Add Button")) {
    // Add a button to the context node
    switch (context_node->type) {
    case NODE_TYPE_VISUAL_PROJECT: {
      mui_button *button;
      mui_init_button(context_node, &button);

      set_c_str(button->str, "button");

    } break;
    default:
      MCerror(9815, "TODO");
    }
    return;
  }
  if (!strcmp(selected_option, "New Module...")) {
    // Lets only have one project at a time for the time being -- TODO
    bool visual_app_exists = false;
    for (int a = 0; a < global_data->children->count; ++a) {
      if (global_data->children->items[a]->type == NODE_TYPE_VISUAL_PROJECT) {
        visual_app_exists = true;
        break;
      }
    }

    if (!visual_app_exists)
      mca_create_new_visual_project("PushTheButton");
  }
  if (!strcmp(selected_option, "Change Button Text To 'click me!'")) {
    // Lets only have one project at a time for the time being -- TODO
    mui_ui_element *element = (mui_ui_element *)context_node->data;
    mui_button *button = (mui_button *)element->data;

    set_c_str(button->str, "click me!");
    mca_set_node_requires_layout_update(element->visual_node);
  }
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

  // TODO CONTEXT OPTIONS
  {
    mca_global_context_node_option_list *options_list =
        (mca_global_context_node_option_list *)malloc(sizeof(mca_global_context_node_option_list));
    options_list->node_type = NODE_TYPE_GLOBAL_ROOT;

    mca_global_context_node_option *option =
        (mca_global_context_node_option *)malloc(sizeof(mca_global_context_node_option));
    option->option_text = strdup("New Module");
    append_to_collection((void ***)&options_list->items, &options_list->alloc, &options_list->count, option);

    option = (mca_global_context_node_option *)malloc(sizeof(mca_global_context_node_option));
    option->option_text = strdup("Cancel");
    append_to_collection((void ***)&options_list->items, &options_list->alloc, &options_list->count, option);
  }
}

void mca_render_global_context_menu(image_render_queue *render_queue, mc_node *node) {}

void mca_gcm_handler(mc_node event_node) { printf("!!!!It Registered!!!!!\n"); }

void mca_activate_global_context_menu(mc_node *context_node, int screen_x, int screen_y)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Show
  // printf("mca_activate_global_context_menu--node %p\n", node);

  mui_ui_element *gcm_element = (mui_ui_element *)global_data->ui_state->global_context_menu->data;
  mui_context_menu_clear_options(gcm_element);

  // Alter available options depending on the type of context_node activated on
  switch (context_node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    mui_context_menu_add_option(gcm_element, "New Module...");
    mui_context_menu_add_option(gcm_element, "Cancel");
  } break;
  case NODE_TYPE_VISUAL_PROJECT: {
    mui_context_menu_add_option(gcm_element, "Add Button");
    mui_context_menu_add_option(gcm_element, "Edit Module Details");
    mui_context_menu_add_option(gcm_element, "Cancel");
  } break;
  case NODE_TYPE_UI: {
    mui_ui_element *element = (mui_ui_element *)context_node->data;
    switch (element->type) {
    case UI_ELEMENT_BUTTON: {
      mui_context_menu_add_option(gcm_element, "Change Button Text To 'click me!'");
      mui_context_menu_add_option(gcm_element, "Cancel");
    } break;
    default:
      // Respond with the default options
      mui_context_menu_add_option(gcm_element, "Cancel");
      // mui_context_menu_add_option(gcm_element, "Add Button");
      // TODO make this a (none) disabled button
      break;
    }
  } break;
  default:
    // Don't show
    // TODO make this a (none) disabled button
    mui_context_menu_add_option(gcm_element, "Cancel");
    // // Respond with the default options
    // mui_context_menu_add_option(gcm_element, "Add Button");
    break;
  }

  // Show
  global_data->ui_state->global_context_menu->visible = true;
  global_data->ui_state->global_context_menu_context_node = context_node;

  // Set New Layout
  gcm_element->layout->padding = {(float)screen_x, (float)screen_y, 0, 0};

  mca_set_node_requires_layout_update(gcm_element->visual_node);
}

void mca_hide_global_context_menu()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->ui_state->global_context_menu->visible = true;

  mca_set_node_requires_layout_update(global_data->ui_state->global_context_menu);
}