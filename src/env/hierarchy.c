
#include "core/core_definitions.h"
#include "env/environment_definitions.h"

void exit_app(mc_node *node_scope, int result)
{
  switch (node_scope->type) {
  // case NODE_TYPE_CONSOLE_APP: {
  //   console_app_info *app_info = (console_app_info *)node_scope->extra;
  //   if (node_scope->parent == NULL) {
  //     // Exit the whole application
  //     exit(result); // Maybe rename this or intercept if for use in midge??
  //   }
  //   else {
  //     // Remove this node from the hierarchy
  //     // remove_node_from_hierarchy() // ??? -- or keep it and show it has exited...?
  //   }
  // } break;
  default:
    MCerror(140, "exit_app>Unsupported node type:%i", node_scope->type);
  }
}

void mca_attach_node_to_hierarchy(mc_node *hierarchy_node, mc_node *node_to_attach)
{
  switch (hierarchy_node->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *global_data = (global_root_data *)hierarchy_node->data;
    append_to_collection((void ***)&global_data->children->items, &global_data->children->alloc,
                         &global_data->children->count, node_to_attach);
  } break;
  case NODE_TYPE_UI: {
    MCerror(3569, "Dont do this, this way... ?");
  } break;
  case NODE_TYPE_VISUAL_PROJECT: {
    visual_project_data *project = (visual_project_data *)hierarchy_node->data;
    append_to_collection((void ***)&project->children->items, &project->children->alloc, &project->children->count,
                         node_to_attach);
  } break;
  default:
    MCerror(3565, "mca_attach_node_to_hierarchy>Unsupported node type:%i", hierarchy_node->type);
  }

  node_to_attach->parent = hierarchy_node;
  // // printf("mca_attach_node_to_hierarchy\n");
  // append_to_collection((void ***)&parent_attachment->children, &parent_attachment->children_alloc,
  //                      &parent_attachment->child_count, node_to_add);

  // // Fire an event...
  // unsigned int event_type = ME_NODE_HIERARCHY_UPDATED;
  // // printf("mca_attach_node_to_hierarchy-2\n");
  // notify_handlers_of_event(event_type, NULL);
  // // printf("mca_attach_node_to_hierarchy-3\n");

  // // TODO -- maybe find a better place to do this
  // switch (node_to_add->type) {
  // case NODE_TYPE_CONSOLE_APP: {
  //   // printf("mca_attach_node_to_hierarchy-4\n");
  //   console_app_info *app_info = (console_app_info *)node_to_add->extra;
  //   if (app_info->initialize_app) {
  //     void *vargs[1];
  //     vargs[0] = &node_to_add;
  //     // printf("mca_attach_node_to_hierarchy-5\n");
  //     // printf("app_info:%p\n", app_info);
  //     // printf("app_info->initialize_app:%p\n", app_info->initialize_app);
  //     // printf("app_info->initialize_app->ptr_declaration:%p\n", app_info->initialize_app->ptr_declaration);
  //     // printf("*app_info->initialize_app->ptr_declaration:%p\n", *(app_info->initialize_app->ptr_declaration));
  //     // printf("**app_info->initialize_app->ptr_declaration:%p\n", **(app_info->initialize_app->ptr_declaration));
  //     (*app_info->initialize_app->ptr_declaration)(1, vargs);
  //     // printf("mca_attach_node_to_hierarchy-6\n");
  //   }
  // } break;
  // default:
  //   break;
  // }
  // printf("mca_attach_node_to_hierarchy-9\n");
}

void mca_init_mc_node(mc_node *hierarchy_node, node_type type, mc_node **node)
{
  (*node) = (mc_node *)malloc(sizeof(mc_node));

  mca_attach_node_to_hierarchy(hierarchy_node, *node);
  (*node)->type = type;

  (*node)->data = NULL;
}

void mca_update_node_list(mc_node_list *node_list)
{
  for (int nl_index = 0; nl_index < node_list->count; ++nl_index) {
    mc_node *node = node_list->items[nl_index];
    switch (node->type) {
    case NODE_TYPE_VISUAL_PROJECT: {
      // Update despite requirements
      mca_update_visual_project(node);
    } break;
    case NODE_TYPE_UI: {
      mui_ui_element *element = (mui_ui_element *)node->data;
      if (!element->requires_update)
        break;

      // UI Update
      // TODO

      // TODO -- this should maybe be somewhere else?
      element->requires_update = false;

      // Trigger rerender
      mca_set_node_requires_rerender(node);
    } break;
    default:
      MCerror(9617, "mca_update_node_list::Unsupported node type:%i", node->type);
    }
  }
}

void mca_render_node_list_headless(mc_node_list *node_list)
{
  for (int a = 0; a < node_list->count; ++a) {
    mc_node *node = node_list->items[a];
    switch (node->type) {
    case NODE_TYPE_UI: {
      mui_render_element_headless(node);
    } break;
    case NODE_TYPE_VISUAL_PROJECT: {
      mca_render_project_headless(node);
    } break;
    default:
      MCerror(1837, "mca_update_headless_render_images>|Unsupported node type:%i", node->type);
    }
  }
}

void mca_render_node_list_present(image_render_queue *render_queue, mc_node_list *node_list)
{
  for (int a = 0; a < node_list->count; ++a) {
    mc_node *node = node_list->items[a];
    switch (node->type) {
    case NODE_TYPE_UI: {
      mui_render_element_present(render_queue, node);
    } break;
    case NODE_TYPE_VISUAL_PROJECT: {
      mca_render_project_present(render_queue, node);
    } break;
    default:
      MCerror(1837, "mca_render_node_list_present>|Unsupported node type:%i", node->type);
    }
  }
}

void mca_set_node_requires_update(mc_node *node)
{
  // Set update required on all ancestors of the node
  while (node) {
    switch (node->type) {
    case NODE_TYPE_GLOBAL_ROOT: {
      // No need to set for global root
    } break;
    case NODE_TYPE_VISUAL_PROJECT: {
      visual_project_data *project = (visual_project_data *)node->data;
      project->ui_state->requires_update = true;
    } break;
    case NODE_TYPE_UI: {
      // printf("mseu-4\n");
      mui_ui_element *element = (mui_ui_element *)node->data;
      element->requires_update = true;
    } break;
    default:
      MCerror(1252, "mca_set_node_requires_update::>unsupported node type:%i", node->type);
    }

    node = node->parent;
  }
}

void mca_set_node_requires_rerender(mc_node *node)
{
  // Set update required on all ancestors of the node
  while (node) {
    switch (node->type) {
    case NODE_TYPE_GLOBAL_ROOT: {
      global_root_data *global_data = (global_root_data *)node->data;
      global_data->requires_rerender = true;
    } break;
    case NODE_TYPE_VISUAL_PROJECT: {
      visual_project_data *project = (visual_project_data *)node->data;
      project->requires_rerender = true;
    } break;
    case NODE_TYPE_UI: {
      // printf("mseu-4\n");
      mui_ui_element *element = (mui_ui_element *)node->data;
      element->requires_rerender = true;
    } break;
    default:
      MCerror(148, "mui_set_element_update::>unsupported node type:%i", node->type);
    }

    node = node->parent;
  }
}

// void add_notification_handler(mc_node *apex_node, unsigned int event_type, int (**handler)(int, void **))
// {
//   event_handler_array *handler_array = NULL;
//   for (int i = 0; i < apex_node->event_handlers.count; ++i) {
//     if (apex_node->event_handlers.items[i]->event_type == event_type) {
//       handler_array = apex_node->event_handlers.items[i];
//       break;
//     }
//   }

//   if (handler_array == NULL) {
//     // Make a new one
//     handler_array = (event_handler_array *)malloc(sizeof(event_handler_array));
//     handler_array->alloc = 0;
//     handler_array->count = 0;
//     handler_array->event_type = event_type;

//     append_to_collection((void ***)&apex_node->event_handlers.items, &apex_node->event_handlers.alloc,
//                          &apex_node->event_handlers.count, handler_array);
//   }

//   // printf("adding %p *->%p\n", handler, *handler);
//   append_to_collection((void ***)&handler_array->handlers, &handler_array->alloc, &handler_array->count, handler);
// }

// void notify_handlers_of_event(unsigned int event_type, void *event_data)
// {
//   // printf("notify_handlers_of_event\n");
//   event_handler_array *handler_array = NULL;
//   for (int i = 0; i < command_hub->global_node->event_handlers.count; ++i) {
//     if (command_hub->global_node->event_handlers.items[i]->event_type == event_type) {
//       handler_array = command_hub->global_node->event_handlers.items[i];
//       break;
//     }
//   }

//   if (handler_array == NULL) {
//     printf("handler_array couldnt be found for:%i out of %i events handled for\n", event_type,
//            command_hub->global_node->event_handlers.count);
//     return;
//   }

//   // printf("hel %i [0]:%p\n", handler_array->count, handler_array->handlers[0]);

//   for (int i = 0; i < handler_array->count; ++i) {
//     if ((*handler_array->handlers[i])) {
//       void *vargs[1];
//       vargs[0] = &event_data;

//       // printf("invoking [%i]:%p\n", i, (*handler_array->handlers[i]));
//       (*handler_array->handlers[i])(1, vargs);
//     }
//   }
//   // register_midge_error_tag("mcd_on_hierarchy_update(~)");
// }

// void attach_definition_to_hierarchy(mc_node *parent_attachment, char *definition)
// {
//   // append_to_collection((void ***)&parent_attachment->children, &parent_attachment->children_alloc,
//   //                      &parent_attachment->child_count, node_to_add);

//   // Fire an event...
//   unsigned int event_type = ME_NODE_HIERARCHY_UPDATED;
//   notify_handlers_of_event(event_type, NULL);

//   // TODO -- maybe find a better place to do this
//   switch (node_to_add->type) {
//   case NODE_TYPE_CONSOLE_APP: {
//     console_app_info *app_info = (console_app_info *)node_to_add->extra;
//     if (app_info->initialize && (*app_info->initialize)) {
//       void *vargs[1];
//       vargs[0] = &node_to_add;
//       (*app_info->initialize)(1, vargs);
//     }
//   } break;
//   default:
//     break;
//   }
// }