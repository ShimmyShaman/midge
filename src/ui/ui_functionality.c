
#include "env/state_definitions.h"
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
  mcr_obtain_font_resource(&global_data->render_thread->resource_queue, "res/font/DroidSansMono.ttf", 18,
                           &ui_state->default_font_resource);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue.mutex);

  // Set
  *p_ui_state = ui_state;
}

void mui_initialize_core_ui_components()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
}

void mui_update_ui()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  global_data->requires_rerender = true;
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

    if (screen_x < (int)element->bounds.x || screen_y < (int)element->bounds.y ||
        screen_x >= (int)(element->bounds.x + element->bounds.width) ||
        screen_y >= (int)(element->bounds.y + element->bounds.height))
      break;

    // Append children
    switch (element->type) {
    case UI_ELEMENT_TEXT_BLOCK:
      break;
    case UI_ELEMENT_PANEL:
      // TODO
      break;
    default:
      MCerror(115, "_mui_get_interactive_nodes_within_node_at_point::>NODE_TYPE_UI>unsupported element type:%i",
              node->type);
    }

    append_to_collection((void ***)&layered_hit_list->items, &layered_hit_list->alloc, &layered_hit_list->count, node);
  } break;
  case NODE_TYPE_VISUAL_PROJECT: {
    visual_project_data *project = (visual_project_data *)node->data;

    mui_ui_element *container_element = (mui_ui_element *)project->editor_container->data;
    if (screen_x < container_element->bounds.x || screen_y < container_element->bounds.y ||
        screen_x >= container_element->bounds.x + project->screen.width ||
        screen_y >= container_element->bounds.y + project->screen.height)
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
  // switch (ui_node->type) {
  // case NODE_TYPE_GLOBAL_ROOT: {
  //   // global_root_data *global_data = (global_root_data *)node->data;

  //   printf("global_node-left_click\n");
  // } break;
  // default:
  //   MCerror(69, "_mui_get_interactive_nodes_within_node_at_point::>unsupported node type:%i", ui_node->type);
  // }
}

void mui_handle_mouse_right_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled)
{
  switch (ui_node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    // global_root_data *global_data = (global_root_data *)ui_node->data;
    // TODO
  } break;
  case NODE_TYPE_VISUAL_PROJECT: {

  } break;
  default:
    MCerror(83, "_mui_get_interactive_nodes_within_node_at_point::>unsupported node type:%i", ui_node->type);
  }
}

void mui_init_ui_element(mc_node *parent_node, ui_element_type element_type, mui_ui_element **created_element)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Node
  mc_node *node = (mc_node *)malloc(sizeof(mc_node));

  node->type = NODE_TYPE_UI;
  if (parent_node->type == NODE_TYPE_UI) {
    mui_ui_element *parent_element = (mui_ui_element *)parent_node->data;
    switch (parent_element->type) {
    case UI_ELEMENT_PANEL: {
      mui_panel *panel = (mui_panel *)parent_element->data;

      append_to_collection((void ***)&panel->children->items, &panel->children->alloc, &panel->children->count, node);
      node->parent = parent_node;

    } break;
    default: {
      MCerror(180, "add element to parent element : Unsupported type : %i", parent_element->type);
    }
    }
  }
  else {
    mca_attach_node_to_hierarchy(parent_node, node);
  }
  // // pthread_mutex_lock(&global_data->uid_counter.mutex);
  // // node->uid = global_data->uid_counter.uid_index++;
  // // pthread_mutex_unlock(&global_data->uid_counter.mutex);

  // UI Element
  mui_ui_element *element = (mui_ui_element *)malloc(sizeof(mui_ui_element));
  node->data = element;

  element->bounds = {};
  element->visual_node = node;
  element->type = element_type;
  element->requires_update = true;
  element->requires_rerender = false;

  element->data = NULL;

  if (created_element)
    *created_element = element;
}