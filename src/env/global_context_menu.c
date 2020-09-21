
#include "env/environment_definitions.h"
#include "ui/ui_definitions.h"

void mca_handle_global_context_menu_option_selected(const char *selected_option);

void mca_init_global_context_menu()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_context_menu *context_menu;
  mui_init_context_menu(global_data->global_node, &context_menu);

  // Set to global
  global_data->ui_state->global_context_menu.node = context_menu->node;
  global_data->ui_state->global_context_menu.context_node = NULL;
  global_data->ui_state->global_context_menu.context_options.alloc = 0;
  global_data->ui_state->global_context_menu.context_options.count = 0;
  global_data->ui_state->global_context_menu.context_options.items = NULL;

  context_menu->node->layout->visible = false;

  context_menu->node->layout->z_layer_index = 10;
  context_menu->node->layout->padding = {150, 200, 0, 0};
  context_menu->node->layout->preferred_width = 120.f;
  context_menu->node->layout->preferred_height = 90.f;
  context_menu->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  context_menu->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  context_menu->background_color = COLOR_DARK_SLATE_GRAY;
  context_menu->option_selected = (void *)&mca_handle_global_context_menu_option_selected;

  mca_set_node_requires_layout_update(context_menu->node);
}

void mca_handle_global_context_menu_option_selected(const char *selected_option)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
  mc_node *context_node = global_data->ui_state->global_context_menu.context_node;
  mc_point context_location = global_data->ui_state->global_context_menu.context_location;

  if (!global_data->ui_state->global_context_menu.context_node) {
    printf("ERROR(non-fatal) global-context-menu selected but no context node exists");
    return;
  }

  if (!strcmp(selected_option, "Cancel")) {
    return;
  }

  mca_global_context_node_option_list *options_list = NULL;
  for (int i = 0; i < global_data->ui_state->global_context_menu.context_options.count; ++i) {
    if (global_data->ui_state->global_context_menu.context_options.items[i]->node_type == context_node->type) {
      options_list = global_data->ui_state->global_context_menu.context_options.items[i];
      break;
    }
  }
  if (!options_list) {
    // TODO??
    printf("ERROR(non-fatal): Couldn't find option for given context-node(%i) and selected option(%s)",
           context_node->type, selected_option);
    return;
  }

  for (int i = 0; i < options_list->count; ++i) {
    if (!strcmp(options_list->items[i]->option_text, selected_option)) {
      void (*event_handler)(mc_node *, mc_point, const char *) =
          (void (*)(mc_node *, mc_point, const char *))options_list->items[i]->event_handler;
      event_handler(context_node, context_location, selected_option);
      return;
    }
  }

  printf(
      "ERROR(non-fatal): Couldn't find selected option text match for given context-node(%i) and selected option(%s)",
      context_node->type, selected_option);
}

void mca_global_context_menu_create_context_list(node_type node_type,
                                                 mca_global_context_node_option_list **context_list)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Argument check for duplicates
  for (int i = 0; i < global_data->ui_state->global_context_menu.context_options.count; ++i) {
    if (global_data->ui_state->global_context_menu.context_options.items[i]->node_type == node_type) {
      MCerror(7874, "context menu options list already exists for node-type:%i", node_type);
    }
  }

  mca_global_context_node_option_list *list =
      (mca_global_context_node_option_list *)malloc(sizeof(mca_global_context_node_option_list));
  list->node_type = node_type;

  list->alloc = 0;
  list->count = 0;
  list->items = NULL;

  mc_node *gcm_node = global_data->ui_state->global_context_menu.node;

  append_to_collection((void ***)&global_data->ui_state->global_context_menu.context_options.items,
                       &global_data->ui_state->global_context_menu.context_options.alloc,
                       &global_data->ui_state->global_context_menu.context_options.count, list);

  *context_list = list;
}

void mca_global_context_menu_add_option_to_node_context(
    node_type node_type, const char *option_text,
    /*void (*event_handler)(mc_node *, const char *)*/ void *event_handler)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Obtain list, or create a new one
  mca_global_context_node_option_list *options_list = NULL;
  for (int i = 0; i < global_data->ui_state->global_context_menu.context_options.count; ++i) {
    if (global_data->ui_state->global_context_menu.context_options.items[i]->node_type == node_type) {
      options_list = global_data->ui_state->global_context_menu.context_options.items[i];
      break;
    }
  }
  if (!options_list) {
    mca_global_context_menu_create_context_list(node_type, &options_list);
  }

  // Add
  mca_global_context_node_option *option =
      (mca_global_context_node_option *)malloc(sizeof(mca_global_context_node_option));
  option->option_text = strdup(option_text);
  option->event_handler = (void *)event_handler;
  append_to_collection((void ***)&options_list->items, &options_list->alloc, &options_list->count, option);
}

// void mca_global_context_menu_create_context_option(mca_global_context_node_option_list *context_list,)
// {

// }

void mca_activate_global_context_menu(mc_node *context_node, int screen_x, int screen_y)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_context_menu *context_menu = (mui_context_menu *)global_data->ui_state->global_context_menu.node->data;
  mui_context_menu_clear_options(context_menu);

  printf("options_list count:%i\n", global_data->ui_state->global_context_menu.context_options.count);
  // Obtain list for node type
  mca_global_context_node_option_list *options_list = NULL;
  for (int i = 0; i < global_data->ui_state->global_context_menu.context_options.count; ++i) {
    if (global_data->ui_state->global_context_menu.context_options.items[i]->node_type == context_node->type) {
      options_list = global_data->ui_state->global_context_menu.context_options.items[i];
      break;
    }
  }
  printf("options_list %s for %i\n", options_list ? "FOUND" : "MISSING", context_node->type);

  if (options_list) {
    for (int i = 0; i < options_list->count; ++i) {
      mui_context_menu_add_option(context_menu, options_list->items[i]->option_text);
    }
  }
  mui_context_menu_add_option(context_menu, "Cancel");

  // Show
  global_data->ui_state->global_context_menu.node->layout->visible = true;
  global_data->ui_state->global_context_menu.context_node = context_node;
  global_data->ui_state->global_context_menu.context_location = {screen_x, screen_y};

  // Set New Layout
  context_menu->node->layout->padding = {(float)screen_x, (float)screen_y, 0, 0};

  mca_set_node_requires_layout_update(context_menu->node);
}

void mca_hide_global_context_menu()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->ui_state->global_context_menu.node->layout->visible = true;

  mca_set_node_requires_layout_update(global_data->ui_state->global_context_menu.node);
}