
#include "env/environment_definitions.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mui_initialize_ui_state(mui_ui_state **p_ui_state)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mui_ui_state *ui_state = (mui_ui_state *)malloc(sizeof(mui_ui_state));

  // cache_layered_hit_list
  ui_state->cache_layered_hit_list = (mc_node_list *)malloc(sizeof(mc_node_list));
  ui_state->cache_layered_hit_list->alloc = 16;
  ui_state->cache_layered_hit_list->count = 0;
  ui_state->cache_layered_hit_list->items =
      (mc_node **)malloc(sizeof(mc_node *) * ui_state->cache_layered_hit_list->alloc);
  // printf("@creation global_data->ui_state:%p\n", global_data->ui_state);

  ui_state->default_font_resource = 0;
  ui_state->requires_update = true;

  // Resource loading
  pthread_mutex_lock(&global_data->render_thread->resource_queue.mutex);

  // Font
  mcr_obtain_font_resource(&global_data->render_thread->resource_queue, "res/font/DroidSansMono.ttf", 18.f,
                           &ui_state->default_font_resource);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue.mutex);

  // Set
  *p_ui_state = ui_state;
}

void mui_initialize_core_ui_components() {}

void mui_update_element_layout(mui_ui_element *element)
{
  // global_root_data *global_data;
  // obtain_midge_global_root(&global_data);

  // global_data->requires_rerender = true;
}

void _mui_get_interactive_nodes_within_node_at_point(mc_node *node, int screen_x, int screen_y,
                                                     mc_node_list *layered_hit_list)
{
  // Including the node itself **
  switch (node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *global_data = (global_root_data *)node->data;

    if (screen_x < 0 || screen_y < 0 || screen_x >= global_data->screen.width || screen_y >= global_data->screen.height)
      break;

    // Add any children before
    for (int a = 0; a < global_data->children->count; ++a) {
      _mui_get_interactive_nodes_within_node_at_point(global_data->children->items[a], screen_x, screen_y,
                                                      layered_hit_list);
    }

    append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count, node);
  } break;
  case NODE_TYPE_UI: {
    mui_ui_element *element = (mui_ui_element *)node->data;

    if (screen_x < (int)element->layout->__bounds.x || screen_y < (int)element->layout->__bounds.y ||
        screen_x >= (int)(element->layout->__bounds.x + element->layout->__bounds.width) ||
        screen_y >= (int)(element->layout->__bounds.y + element->layout->__bounds.height))
      break;

    // Append children
    switch (element->type) {
    case UI_ELEMENT_TEXT_BLOCK:
    case UI_ELEMENT_BUTTON:
      break;
    case UI_ELEMENT_CONTEXT_MENU: {
      mui_context_menu *context_menu = (mui_context_menu *)element->data;
      for (int a = 0; a < context_menu->children->count; ++a) {
        _mui_get_interactive_nodes_within_node_at_point(context_menu->children->items[a], screen_x, screen_y,
                                                        layered_hit_list);
      }
    } break;
    case UI_ELEMENT_PANEL: {
      mui_panel *panel = (mui_panel *)element->data;
      for (int a = 0; a < panel->children->count; ++a) {
        _mui_get_interactive_nodes_within_node_at_point(panel->children->items[a], screen_x, screen_y,
                                                        layered_hit_list);
      }
    } break;
    default:
      MCerror(115, "_mui_get_interactive_nodes_within_node_at_point::>NODE_TYPE_UI>unsupported element type:%i",
              node->type);
    }

    append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count, node);
  } break;
  case NODE_TYPE_VISUAL_PROJECT: {
    visual_project_data *project = (visual_project_data *)node->data;

    mui_ui_element *container_element = (mui_ui_element *)project->editor_container->data;
    if (screen_x < container_element->layout->__bounds.x || screen_y < container_element->layout->__bounds.y ||
        screen_x >= container_element->layout->__bounds.x + project->screen.width ||
        screen_y >= container_element->layout->__bounds.y + project->screen.height)
      break;

    // Add any children before
    for (int a = 0; a < project->children->count; ++a) {
      _mui_get_interactive_nodes_within_node_at_point(project->children->items[a], screen_x, screen_y,
                                                      layered_hit_list);
    }

    append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count, node);
  } break;
  default:
    MCerror(1227, "_mui_get_interactive_nodes_within_node_at_point::>unsupported node type:%i", node->type);
  }
}

// Returns a list of ui-type nodes at the given point of the screen. Nodes at the nearer Z are earlier in the list.
void mui_get_interactive_nodes_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Use the cache list
  // printf("layered_hit_list:%p\n", layered_hit_list);
  // printf("global_data->ui_state:%p\n", global_data->ui_state);
  // printf("global_data->ui_state->cache_layered_hit_list:%p\n", global_data->ui_state->cache_layered_hit_list);
  *layered_hit_list = global_data->ui_state->cache_layered_hit_list;
  (*layered_hit_list)->count = 0;

  _mui_get_interactive_nodes_within_node_at_point(global_data->global_node, screen_x, screen_y, *layered_hit_list);

  printf("mui_get_interactive_nodes_at_point(%i, %i) : list_count:%i\n", screen_x, screen_y,
         (*layered_hit_list)->count);
}

void mui_handle_mouse_left_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled)
{
  switch (ui_node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    // global_root_data *global_data = (global_root_data *)node->data;

    // printf("global_node-left_click\n");
  } break;
  case NODE_TYPE_UI: {
    mui_ui_element *element = (mui_ui_element *)ui_node->data;

    switch (element->type) {
    case UI_ELEMENT_BUTTON: {
      mui_button *button = (mui_button *)element->data;
      if (button->left_click) {
        // TODO -- better function pointer transcription
        // void (*event_handler)(int, void **) = (void (*)(int, void **))button->left_click;
        // void *args[2];
        // int b = 80085;
        // args[0] = &b;
        // event_handler(2, args);

        // void (*event_handler)(int) = (void (*)(int))button->left_click;
        // printf("event_handler@hmlc:%p\n", event_handler);
        // event_handler(4241);

        // DEBUG TODO -- has to be casted to local fptr first
        void (*event_handler)(mui_button *, mc_point) = (void (*)(mui_button *, mc_point))button->left_click;
        event_handler(button, (mc_point){screen_x, screen_y});
      }
      *handled = true;
    } break;
    case UI_ELEMENT_CONTEXT_MENU: {
      // Do nothing
      *handled = true;
    } break;
    default:
      MCerror(9155, "_mui_get_interactive_nodes_within_node_at_point::>unsupported element type:%i", element->type);
    }
  } break;
  case NODE_TYPE_VISUAL_PROJECT: {
    // Nothing
    // Maybe editor container ? TODO
    // printf("NODE_TYPE_VISUAL_PROJECT-left_click\n");
  } break;
  default:
    MCerror(9159, "_mui_get_interactive_nodes_within_node_at_point::>unsupported node type:%i", ui_node->type);
  }
}

void mui_handle_mouse_right_click(mc_node *node, int screen_x, int screen_y, bool *handled)
{
  *handled = false;

  switch (node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    mca_activate_global_context_menu(node, screen_x, screen_y);
    *handled = true;
  } break;
  case NODE_TYPE_VISUAL_PROJECT: {
    mca_activate_global_context_menu(node, screen_x, screen_y);
    *handled = true;
  } break;
  case NODE_TYPE_UI: {
    printf("Node_type_interaction skipped\n");
  } break;
  default:
    MCerror(8315, "_mui_get_interactive_nodes_within_node_at_point::>unsupported node type:%i", node->type);
  }
}

void mui_init_ui_element(mc_node *parent_node, ui_element_type element_type, mui_ui_element **created_element)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Node
  mc_node *node;
  mca_init_mc_node(parent_node, NODE_TYPE_UI, &node);

  // if (parent_node->type == NODE_TYPE_UI) {
  //   mui_ui_element *parent_element = (mui_ui_element *)parent_node->data;
  //   switch (parent_element->type) {
  //   case UI_ELEMENT_PANEL: {
  //     mui_panel *panel = (mui_panel *)parent_element->data;

  //     append_to_collection((void ***)&panel->children->items, &panel->children->alloc, &panel->children->count,
  //     node); node->parent = parent_node;

  //   } break;
  //   case UI_ELEMENT_CONTEXT_MENU: {
  //     mui_context_menu *menu = (mui_context_menu *)parent_element->data;

  //     append_to_collection((void ***)&menu->children->items, &menu->children->alloc, &menu->children->count, node);
  //     node->parent = parent_node;
  //   } break;
  //   default: {
  //     MCerror(1805, "mui_init_ui_element::Unsupported type : %i", parent_element->type);
  //   }
  //   }
  // }
  // else {
  // }
  // // pthread_mutex_lock(&global_data->uid_counter.mutex);
  // // node->uid = global_data->uid_counter.uid_index++;
  // // pthread_mutex_unlock(&global_data->uid_counter.mutex);

  // UI Element
  mui_ui_element *element = (mui_ui_element *)malloc(sizeof(mui_ui_element));
  node->data = element;

  {
    // Initialize layout
    element->layout = (mca_node_layout *)malloc(sizeof(mca_node_layout));

    element->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
    element->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
    element->layout->preferred_width = 0;
    element->layout->preferred_height = 0;
    // element->layout->min_width = 0;
    // element->layout->min_height = 0;
    // element->layout->max_width = 0;
    // element->layout->max_height = 0;
    element->layout->padding = {0, 0, 0, 0};
  }
  element->visual_node = node;
  element->type = element_type;
  element->requires_rerender = false;

  element->data = NULL;

  mca_set_node_requires_layout_update(node);

  if (created_element)
    *created_element = element;
}

void mui_get_hierarchical_children_node_list(mc_node *hierarchy_node, mc_node_list **children_node_list)
{
  mui_ui_element *element = (mui_ui_element *)hierarchy_node->data;

  *children_node_list = NULL;
  switch (element->type) {
  case UI_ELEMENT_PANEL: {
    mui_panel *item = (mui_panel *)element->data;
    *children_node_list = item->children;
  } break;
  case UI_ELEMENT_CONTEXT_MENU: {
    mui_context_menu *item = (mui_context_menu *)element->data;
    *children_node_list = item->children;
  } break;
  default:
    MCerror(8286, "TODO Support %i", element->type);
  }
}