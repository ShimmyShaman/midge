
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "ui/ui_definitions.h"

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

// void mca_logic_update_node_list(mc_node_list *node_list)
// {
//   for (int nl_index = 0; nl_index < node_list->count; ++nl_index) {
//     mc_node *node = node_list->items[nl_index];
//     switch (node->type) {
//     case NODE_TYPE_VISUAL_PROJECT: {
//       // Update despite requirements
//       mca_update_visual_project(node);
//     } break;
//     case NODE_TYPE_UI: {
//       mui_ui_element *element = (mui_ui_element *)node->data;
//       // Nothing for the moment -- TODO?
//     } break;
//     default:
//       MCerror(9617, "mca_update_node_list::Unsupported node type:%i", node->type);
//     }
//   }
// }
// void mca_update_node_layout_location(mc_node *node, mc_rectf *available_area, layout_extent_restraints restraints) {}

// void mca_update_child_node_layout(mc_node *node, mc_rectf *available_area, layout_extent_restraints restraints)
// {
//   switch (node->type) {
//   case NODE_TYPE_UI: {
//     mui_ui_element *element = (mui_ui_element *)node->data;
//     switch (element->type) {
//     case UI_ELEMENT_TEXT_BLOCK: {
//       mca_node_layout *layout = element->layout;

//       // Preferred value > padding (within min/max if set)

//       mc_rectf bounds;

//       // Width
//       if (layout->preferred_width) {
//         // Set to preferred width
//         bounds.width = layout->preferred_width;
//       }
//       else {
//         if (restraints & LAYOUT_RESTRAINT_HORIZONTAL) {
//           if (layout->min_width)
//             bounds.width += layout->min_width;
//           else {
//             layout->__bounds.width = 0;
//             break;
//           }
//         }
//         else {
//           // padding adjusted from available
//           bounds.width = available_area->width - layout->padding.right - layout->padding.left;

//           // Specified bounds
//           if (layout->min_width && bounds.width < layout->min_width) {
//             bounds.width = layout->min_width;
//           }
//           if (layout->max_width && bounds.width > layout->max_width) {
//             bounds.width = layout->max_width;
//           }

//           if (bounds.width < 0) {
//             bounds.width = 0;
//           }
//         }
//       }

//       // Height
//       if (layout->preferred_height) {
//         // Set to preferred height
//         bounds.height = layout->preferred_height;
//       }
//       else {
//         if (restraints & LAYOUT_RESTRAINT_VERTICAL) {
//           if (layout->min_height)
//             bounds.height += layout->min_height;
//           else {
//             layout->__bounds.height = 0;
//             break;
//           }
//         }
//         else {
//           // padding adjusted from available
//           bounds.height = available_area->height - layout->padding.bottom - layout->padding.top;

//           // Specified bounds
//           if (layout->min_height && bounds.height < layout->min_height) {
//             bounds.height = layout->min_height;
//           }
//           if (layout->max_height && bounds.height > layout->max_height) {
//             bounds.height = layout->max_height;
//           }

//           if (bounds.height < 0) {
//             bounds.height = 0;
//           }
//         }
//       }

//       if (!bounds.width || !bounds.height) {
//         layout->__bounds = bounds;
//         break;
//       }

//       // X
//       switch (layout->horizontal_alignment) {
//       case HORIZONTAL_ALIGNMENT_LEFT: {
//         bounds.x = available_area->x + layout->padding.left;
//       } break;
//       case HORIZONTAL_ALIGNMENT_RIGHT: {
//         bounds.x = available_area->x + available_area->width - layout->padding.right - bounds.width;
//       } break;
//       case HORIZONTAL_ALIGNMENT_CENTRED: {
//         bounds.x = available_area->x + layout->padding.left +
//                    (available_area->width - layout->padding.right - bounds.width) / 2.f;
//       } break;
//       default:
//         MCerror(7180, "NotSupported:%i", layout->horizontal_alignment);
//       }

//       // Y
//       switch (layout->vertical_alignment) {
//       case VERTICAL_ALIGNMENT_TOP: {
//         bounds.y = available_area->y + layout->padding.top;
//       } break;
//       case VERTICAL_ALIGNMENT_BOTTOM: {
//         bounds.y = available_area->y + available_area->height - layout->padding.bottom - bounds.height;
//       } break;
//       case VERTICAL_ALIGNMENT_CENTRED: {
//         bounds.y = available_area->y + layout->padding.top +
//                    (available_area->height - layout->padding.bottom - bounds.height) / 2.f;
//       } break;
//       default:
//         MCerror(7195, "NotSupported:%i", layout->vertical_alignment);
//       }

//       printf("bounds = {%.3f, %.3f, %.3f, %.3f}\n", bounds.x, bounds.y, bounds.width, bounds.height);

//       if (bounds.x != layout->__bounds.x || bounds.y != layout->__bounds.y || bounds.width != layout->__bounds.width
//       ||
//           bounds.height != layout->__bounds.height) {
//         layout->__bounds = bounds;
//         mca_set_node_requires_rerender(node);
//       }
//     } break;
//     case UI_ELEMENT_CONTEXT_MENU: {
//       // mui_update_context_menu_layout(node, available_area, restraints);
//       mui_context_menu *context_menu = (mui_context_menu *)element->data;

//       // Determine the maximum width requested by child controls and the cumulative height

//       // Ensure they lie within min & max width parameters

//       // Set accordingly
//     } break;
//     default:
//       MCerror(9117, "mca_update_node_layout_extents::Unsupported element type:%i", element->type);
//     }
//   } break;
//   default:
//     MCerror(9121, "mca_update_node_layout_extents::Unsupported node type:%i", node->type);
//   }
// }

void mca_update_list_nodes_layout_extents(mc_node_list *node_list, layout_extent_restraints restraints)
{
  for (int a = 0; a < node_list->count; ++a) {
    mc_node *node = node_list->items[a];

    // printf("mca_update_node_layout--\n");
    switch (node->type) {
    case NODE_TYPE_UI: {
      mui_ui_element *element = (mui_ui_element *)node->data;
      if (!element->requires_layout_update)
        continue;

      switch (element->type) {
      case UI_ELEMENT_TEXT_BLOCK: {
        mui_text_block *text_block = (mui_text_block *)element->data;

        float str_width, str_height;
        mcr_determine_text_display_dimensions(text_block->font_resource_uid, text_block->str->text, &str_width,
                                              &str_height);

        // Width
        if (element->layout->preferred_width)
          element->layout->__bounds.width = element->layout->preferred_width;
        else
          element->layout->__bounds.width = str_width;

        // Height
        if (element->layout->preferred_height)
          element->layout->__bounds.height = element->layout->preferred_height;
        else
          element->layout->__bounds.height = str_height;

      } break;
      case UI_ELEMENT_BUTTON: {
        if (!element->layout->preferred_width) {
          MCerror(2651, "Not Supported");
        }
        if (!element->layout->preferred_height) {
          MCerror(2652, "Not Supported");
        }

        element->layout->__bounds.width = element->layout->preferred_width;
        element->layout->__bounds.height = element->layout->preferred_height;
      } break;
      case UI_ELEMENT_CONTEXT_MENU: {
        mui_context_menu *context_menu = (mui_context_menu *)element->data;

        // Determine children extents
        mca_update_list_nodes_layout_extents(context_menu->children,
                                             LAYOUT_RESTRAINT_HORIZONTAL | LAYOUT_RESTRAINT_VERTICAL);
        // mca_update_list_nodes_layout(context_menu->children, available_area);

        float max_child_width = 0, cumulative_height = 0;
        for (int a = 0; a < context_menu->_buttons.count; ++a) {
          mui_button *button = context_menu->_buttons.items[a];

          if (button->element->layout->__bounds.width > max_child_width) {
            max_child_width = button->element->layout->padding.left + button->element->layout->__bounds.width +
                              button->element->layout->padding.right;
          }

          cumulative_height += button->element->layout->padding.top + button->element->layout->__bounds.height +
                               button->element->layout->padding.bottom;
        }

        if (element->layout->preferred_width) {
          element->layout->__bounds.width = element->layout->preferred_width;
        }
        else {
          element->layout->__bounds.width = max_child_width;
        }
        if (element->layout->preferred_height) {
          element->layout->__bounds.height = element->layout->preferred_height;
        }
        else {
          element->layout->__bounds.height = cumulative_height;
        }

      } break;
      default:
        MCerror(9268, "mca_update_list_nodes_layout_extents::Unsupported element type:%i", element->type);
      }
    } break;
    default:
      MCerror(9272, "mca_update_list_nodes_layout_extents::Unsupported node type:%i", node->type);
    }
  }
}

void mca_update_node_layout(mc_node *node, mc_rectf *available_area)
// layout_extent_restraints restraints)
{
  // printf("mca_update_node_layout--\n");
  switch (node->type) {
  case NODE_TYPE_UI: {
    mui_ui_element *element = (mui_ui_element *)node->data;
    switch (element->type) {
    case UI_ELEMENT_TEXT_BLOCK: {
      element->layout->__bounds.x = available_area->x + element->layout->padding.left;
      element->layout->__bounds.y = available_area->y + element->layout->padding.top;

    } break;
    case UI_ELEMENT_BUTTON: {
      element->layout->__bounds.x = available_area->x + element->layout->padding.left;
      element->layout->__bounds.y = available_area->y + element->layout->padding.top;

    } break;
    case UI_ELEMENT_CONTEXT_MENU: {
      mui_context_menu *context_menu = (mui_context_menu *)element->data;

      element->layout->__bounds.x = available_area->x + element->layout->padding.left;
      element->layout->__bounds.y = available_area->y + element->layout->padding.top;

      mc_rectf child_bounds = element->layout->__bounds;
      for (int b = 0; b < context_menu->_buttons.count; ++b) {
        mui_button *button = context_menu->_buttons.items[b];
        mca_update_node_layout(button->element->visual_node, &child_bounds);

        child_bounds.y += button->element->layout->padding.top + button->element->layout->__bounds.height +
                          button->element->layout->padding.bottom;
      }
      //   // // Determine children extents
      //   // mca_update_list_nodes_layout_extents(context_menu->children, available_area,
      //   //                                      LAYOUT_RESTRAINT_HORIZONTAL | LAYOUT_RESTRAINT_VERTICAL);
      //   // // mca_update_list_nodes_layout(context_menu->children, available_area);

      //   // float max_child_width = 0, cumulative_height = 0;
      //   // for (int a = 0; a < context_menu->_buttons.count; ++a) {
      //   //   mui_button *button = context_menu->_buttons.items[a];

      //   //   if (button->element->layout->__bounds.width > max_child_width) {
      //   //     max_child_width = button->element->layout->padding.left + button->element->layout->__bounds.width +
      //   //                       button->element->layout->padding.right;
      //   //   }

      //   //   cumulative_height += button->element->layout->padding.top + button->element->layout->__bounds.height +
      //   //                        button->element->layout->padding.bottom;
      //   // }

      //   // if (element->layout->preferred_width) {
      //   //   element->layout->__bounds.width = element->layout->preferred_width;
      //   // }
      //   // else {
      //   //   element->layout->__bounds.width = max_child_width;
      //   // }
      //   // if (element->layout->preferred_height) {
      //   //   element->layout->__bounds.height = element->layout->preferred_height;
      //   // }
      //   // else {
      //   //   element->layout->__bounds.height = cumulative_height;
      //   // }

    } break;
    default:
      MCerror(9270, "mca_update_node_layout::Unsupported element type:%i", element->type);
    }
  } break;
    case NODE_TYPE -- Whats 8??
    
    // case NODE_TYPE_GLOBAL_ROOT: {
    //   global_root_data *global_data = (global_root_data *)node->data;

    //   mc_rectf bounds = {0, 0, (float)global_data->screen.width, (float)global_data->screen.height};
    //   for (int a = 0; a < global_data->children->count; ++a) {
    //     // printf("mca_update_node_layout--child %p\n", global_data->children->items[a]);
    //     if (global_data->children->items[a]->visible) {
    //       // printf("mca_update_node_layout--child visible\n");
    //       mca_update_child_node_layout(global_data->children->items[a], &bounds, LAYOUT_RESTRAINT_NONE);
    //     }
    //   }

    // for (int a = 0; a < global_data->children->count; ++a) {
    //   mca_update_node_layout_location(global_data->children->items[a], &bounds);
    // }

    // // Set child positions
    // for (int a = 0; a < global_data->children->count; ++a) {
    //   switch (global_data->children->items[a]->type) {
    //   default:
    //     MCerror(8124, "TODO %i", global_data->children->items[a]->type)
    //   }
    // }

    // mc_rectf bounds = {0, 0, (float)global_data->screen.width, (float)global_data->screen.height};
    // for (int a = 0; a < global_data->children->count; ++a) {
    //   mca_update_node_layout_positions(global_data->children->items[a], &bounds);
    // }
    // }
    // break;
  // case NODE_TYPE_VISUAL_PROJECT: {
  //   // Update despite requirements
  //   mca_update_visual_project(node);
  // } break;
  // case NODE_TYPE_UI: {
  //   mui_ui_element *element = (mui_ui_element *)node->data;
  //   // Nothing for the moment -- TODO?
  // } break;
  default:
    MCerror(9617, "mca_update_node_layout::Unsupported node type:%i", node->type);
  }

  // for (int nl_index = 0; nl_index < node_list->count; ++nl_index) {
  //   mc_node *node = node_list->items[nl_index];
  //   switch (node->type) {
  //   case NODE_TYPE_VISUAL_PROJECT: {
  //     // Update despite requirements
  //     mca_update_visual_project(node);
  //   } break;
  //   case NODE_TYPE_UI: {
  //     mui_ui_element *element = (mui_ui_element *)node->data;
  //     if (!element->requires_layout_update)
  //       break;

  //     // UI Update
  //     mui_update_element_layout(element);

  //     // TODO -- this should maybe be somewhere else?
  //     element->requires_layout_update = false;

  //     // Trigger rerender
  //     mca_set_node_requires_rerender(node);
  //   } break;
  //   default:
  //     MCerror(9613, "mca_update_node_list::Unsupported node type:%i", node->type);
  //   }
  // }
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
    if (!node->visible)
      continue;

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

void mca_set_node_requires_layout_update(mc_node *node)
{
  // Set update required on all ancestors of the node
  while (node) {
    switch (node->type) {
    case NODE_TYPE_GLOBAL_ROOT: {
      global_root_data *global_data = (global_root_data *)node->data;
      global_data->requires_layout_update = true;
    } break;
    case NODE_TYPE_VISUAL_PROJECT: {
      visual_project_data *project = (visual_project_data *)node->data;
      project->ui_state->requires_update = true;
    } break;
    case NODE_TYPE_UI: {
      // printf("mseu-4\n");
      mui_ui_element *element = (mui_ui_element *)node->data;
      element->requires_layout_update = true;
    } break;
    default:
      MCerror(1252, "mca_set_node_requires_layout_update::>unsupported node type:%i", node->type);
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