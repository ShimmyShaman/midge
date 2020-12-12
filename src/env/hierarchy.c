/* hierarchy.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "ui/ui_definitions.h"

#include "core/midge_app.h"

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
    printf("ERR[140]:exit_app>Unsupported node type:%i", node_scope->type);
    exit(-1);
  }
}

// void mca_get_sub_hierarchy_node_list(mc_node *hierarchy_node, mc_node_list **sub_node_list)
// {
//   switch (hierarchy_node->type) {
//   case NODE_TYPE_GLOBAL_ROOT: {
//     mc_app_info *app_info = (mc_app_info *)hierarchy_node->data;
//     *sub_node_list = app_info->children;
//   } break;
//   case NODE_TYPE_UI: {
//     mcu_get_hierarchical_children_node_list(hierarchy_node, sub_node_list);
//   } break;
//   case NODE_TYPE_VISUAL_PROJECT: {
//     visual_project_data *project = (visual_project_data *)hierarchy_node->data;
//     *sub_node_list = project->children;
//   } break;
//   default:
//     MCerror(3565, "mca_get_sub_hierarchy_node_list>Unsupported node type:%i", hierarchy_node->type);
//   }
// }

// void __mca_insert_node_into_node_list(mc_node_list *parent_node_list, mc_node *node_to_insert,
//                                       unsigned int z_layer_index)
// {
//   // Resize node list if need be
//   {
//     if (parent_node_list->count + 1 > parent_node_list->alloc) {
//       unsigned int realloc_amount = parent_node_list->alloc + 8 + parent_node_list->alloc / 3;
//       // printf("reallocate collection size %i->%i\n", parent_node_list->alloc, realloc_amount);
//       mc_node **new_items = (mc_node **)malloc(sizeof(mc_node *) * realloc_amount);
//       unsigned int *new_z_layer_indices = (unsigned int *)malloc(sizeof(unsigned int) * realloc_amount);
//       if (!new_items || !new_z_layer_indices) {
//         MCerror(32, "append_to_collection malloc error");
//       }

//       if (parent_node_list->alloc) {
//         memcpy(new_items, parent_node_list->items, parent_node_list->count * sizeof(mc_node *));
//         free(parent_node_list->items);
//         memcpy(new_z_layer_indices, parent_node_list->items, parent_node_list->count * sizeof(unsigned int));
//         free(parent_node_list->z_layer_indices);
//       }

//       parent_node_list->items = new_items;
//       parent_node_list->z_layer_indices = new_z_layer_indices;
//       parent_node_list->alloc = realloc_amount;
//     }
//   }

//   // Fit the item in where z-appropriate
//   {
//     // Insert
//     int insertion_index = -1;
//     for (int n = parent_node_list->count - 1; n >= 0; --n) {
//       if (z_layer_index >= parent_node_list->z_layer_indices[n]) {
//         insertion_index = n + 1;
//         break;
//       }
//     }

//     if (insertion_index < 0)
//       insertion_index = 0;

//     for (int i = parent_node_list->count; i > insertion_index; --i) {
//       parent_node_list->items[i] = parent_node_list->items[i - 1];
//     }
//     parent_node_list->items[insertion_index] = node_to_insert;
//     parent_node_list->z_layer_indices[insertion_index] = z_layer_index;
//   }

//   // Increment list count
//   ++parent_node_list->count;
// }

int mca_attach_node_to_hierarchy(mc_node *hierarchy_node, mc_node *node_to_attach)
{
  // printf("added node %i to %i\n", node_to_attach->type, hierarchy_node->type);
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  if (!hierarchy_node->children) {
    MCerror(9108, "Attempt to attach node (%i) to parent (%i) who has no children", node_to_attach->type,
            hierarchy_node->type);
  }

  // Lock thread-safety mutex
  // printf("hierarchy-locking... %p\n", &app_info->hierarchy_mutex);
  pthread_mutex_lock(&app_info->hierarchy_mutex);
  // puts("hierarchy-locked");

  // Attach & Update
  node_to_attach->parent = hierarchy_node;
  if (node_to_attach->layout) {
    // Insert it appropriately according to its z-index
    mc_node_list *children = hierarchy_node->children;
    int a;
    mc_node *oc;
    for (a = hierarchy_node->children->count; a > 0; --a) {
      oc = children->items[a - 1];

      if (oc->layout && oc->layout->z_layer_index <= node_to_attach->layout->z_layer_index) {
        MCcall(insert_in_collection((void ***)&children->items, &children->alloc, &children->count, a, node_to_attach));
        break;
      }
    }
    if (!a) {
      MCcall(insert_in_collection((void ***)&children->items, &children->alloc, &children->count, a, node_to_attach));
    }

    // TODO -- not sure this is the best way to handle this yet
    // .. maybe use a layout flag which specifies to update all children as well?
    if (node_to_attach->children)
      mca_set_descendents_require_layout_update(node_to_attach->children);

    mca_set_node_requires_layout_update(node_to_attach);
  }
  else {
    MCcall(append_to_collection((void ***)&hierarchy_node->children->items, &hierarchy_node->children->alloc,
                                &hierarchy_node->children->count, node_to_attach));
  }
  mca_set_node_requires_rerender(node_to_attach);

  pthread_mutex_unlock(&app_info->hierarchy_mutex);
  // puts("hierarchy-unlock");

  return 0;
}

int mca_init_node_layout(mca_node_layout **layout)
{
  // Initialize layout
  (*layout) = (mca_node_layout *)malloc(sizeof(mca_node_layout));
  MCassert(*layout, "Failure to allocate memory");

  (*layout)->visible = true;
  (*layout)->focused_child = NULL;

  (*layout)->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  (*layout)->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
  (*layout)->preferred_width = 0;
  (*layout)->preferred_height = 0;
  (*layout)->min_width = 0;
  (*layout)->min_height = 0;
  (*layout)->max_width = 0;
  (*layout)->max_height = 0;
  (*layout)->padding = (mc_paddingf){0, 0, 0, 0};

  (*layout)->z_layer_index = 5U;

  (*layout)->handle_input_event = NULL;

  (*layout)->determine_layout_extents = NULL;
  (*layout)->update_layout = NULL;
  (*layout)->render_headless = NULL;
  (*layout)->render_present = NULL;

  (*layout)->__requires_layout_update = true;
  (*layout)->__requires_rerender = false;

  return 0;
}

int mca_init_mc_node(node_type type, const char *name, mc_node **node)
{
  (*node) = (mc_node *)malloc(sizeof(mc_node)); // TODO malloc checks everywhere?

  (*node)->type = type;
  (*node)->name = name ? strdup(name) : NULL;

  (*node)->layout = NULL;
  (*node)->children = NULL;

  (*node)->data = NULL;
  (*node)->parent = NULL;

  return 0;
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
//       mcu_ui_element *element = (mcu_ui_element *)node->data;
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
//     mcu_ui_element *element = (mcu_ui_element *)node->data;
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
//       // mcu_update_context_menu_layout(node, available_area, restraints);
//       mcu_context_menu *context_menu = (mcu_context_menu *)element->data;

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

int mca_determine_typical_node_extents(mc_node *node, layout_extent_restraints restraints)
{
  const float MAX_EXTENT_VALUE = 100000.f;
  mca_node_layout *layout = node->layout;

  // Width
  if (layout->preferred_width) {
    // Set to preferred width
    layout->determined_extents.width = layout->preferred_width;
  }
  else {
    if (restraints & LAYOUT_RESTRAINT_HORIZONTAL) {
      if (layout->min_width)
        layout->determined_extents.width = layout->min_width;
      else {
        layout->determined_extents.width = 0;
      }
    }
    else {
      // padding adjusted from available
      layout->determined_extents.width = MAX_EXTENT_VALUE;

      // Specified bounds
      if (layout->min_width && layout->determined_extents.width < layout->min_width) {
        layout->determined_extents.width = layout->min_width;
      }
      if (layout->max_width && layout->determined_extents.width > layout->max_width) {
        layout->determined_extents.width = layout->max_width;
      }

      if (layout->determined_extents.width < 0) {
        layout->determined_extents.width = 0;
      }
    }
  }

  // Height
  if (layout->preferred_height) {
    // Set to preferred height
    layout->determined_extents.height = layout->preferred_height;
  }
  else {
    if (restraints & LAYOUT_RESTRAINT_VERTICAL) {
      if (layout->min_height)
        layout->determined_extents.height = layout->min_height;
      else {
        layout->determined_extents.height = 0;
      }
    }
    else {
      // padding adjusted from available
      layout->determined_extents.height = MAX_EXTENT_VALUE;

      // Specified bounds
      if (layout->min_height && layout->determined_extents.height < layout->min_height) {
        layout->determined_extents.height = layout->min_height;
      }
      if (layout->max_height && layout->determined_extents.height > layout->max_height) {
        layout->determined_extents.height = layout->max_height;
      }

      if (layout->determined_extents.height < 0) {
        layout->determined_extents.height = 0;
      }
    }
  }

  // Children
  if (node->children) {
    for (int a = 0; a < node->children->count; ++a) {
      mc_node *child = node->children->items[a];
      if (child->layout && child->layout->determine_layout_extents) {
        // TODO fptr casting
        void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
            (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
        determine_layout_extents(child, LAYOUT_RESTRAINT_NONE);
      }
    }
  }

  return 0;
}

// TODO -- make this return int success and all delegates etc
int mca_update_typical_node_layout(mc_node *node, mc_rectf const *available_area)
{
  // Clear
  node->layout->__requires_layout_update = false;

  // Preferred value > padding (within min/max if set)
  mc_rectf bounds;
  mca_node_layout *layout = node->layout;
  layout->__requires_layout_update = false;

  // Width
  if (layout->preferred_width) {
    // Set to preferred width
    bounds.width = layout->preferred_width;
  }
  else {
    // padding adjusted from available
    bounds.width = available_area->width - layout->padding.right - layout->padding.left;

    // Apply the determined extent
    if (bounds.width > layout->determined_extents.width) {
      bounds.width = layout->determined_extents.width;
    }
  }

  // Height
  if (layout->preferred_height) {
    // Set to preferred height
    bounds.height = layout->preferred_height;
    // printf("preferred\n");
  }
  else {
    // printf("available_area->height:%.3f layout->padding.bottom:%.3f layout->padding.top:%.3f\n",
    // available_area->height,
    //        layout->padding.bottom, layout->padding.top);
    // padding adjusted from available
    bounds.height = available_area->height - layout->padding.bottom - layout->padding.top;

    // Apply the determined extent
    if (bounds.height > layout->determined_extents.height) {
      bounds.height = layout->determined_extents.height;
    }
  }

  // X
  switch (layout->horizontal_alignment) {
  case HORIZONTAL_ALIGNMENT_LEFT: {
    // printf("left %.3f %.3f\n", available_area->x, layout->padding.left);
    bounds.x = available_area->x + layout->padding.left;
  } break;
  case HORIZONTAL_ALIGNMENT_RIGHT: {
    // printf("right %.3f %.3f %.3f %.3f\n", available_area->x, layout->padding.left, layout->padding.right,
    // bounds.width);
    bounds.x = available_area->x + available_area->width - layout->padding.right - bounds.width;
  } break;
  case HORIZONTAL_ALIGNMENT_CENTRED: {
    // printf("centred %.3f %.3f %.3f %.3f %.3f\n", available_area->x, layout->padding.left, available_area->width,
    //  layout->padding.right, bounds.width);
    bounds.x = available_area->x + layout->padding.left +
               (available_area->width - (layout->padding.left + bounds.width + layout->padding.right)) / 2.f;
  } break;
  default:
    MCerror(7371, "NotSupported:%i", layout->horizontal_alignment);
  }

  // Y
  switch (layout->vertical_alignment) {
  case VERTICAL_ALIGNMENT_TOP: {
    bounds.y = available_area->y + layout->padding.top;
  } break;
  case VERTICAL_ALIGNMENT_BOTTOM: {
    bounds.y = available_area->y + available_area->height - layout->padding.bottom - bounds.height;
  } break;
  case VERTICAL_ALIGNMENT_CENTRED: {
    bounds.y = available_area->y + layout->padding.top +
               (available_area->height - (layout->padding.bottom + bounds.height + layout->padding.top)) / 2.f;
  } break;
  default:
    MCerror(7387, "NotSupported:%i", layout->vertical_alignment);
  }

  // Set if different
  if (bounds.x != layout->__bounds.x || bounds.y != layout->__bounds.y || bounds.width != layout->__bounds.width ||
      bounds.height != layout->__bounds.height) {
    layout->__bounds = bounds;
    // printf("setrerender\n");
    mca_set_node_requires_rerender(node);
  }

  // Children
  if (node->children) {
    for (int a = 0; a < node->children->count; ++a) {
      mc_node *child = node->children->items[a];
      if (child->layout && child->layout->update_layout) {
        // TODO fptr casting
        void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
        update_layout(child, &layout->__bounds);
      }
    }
  }

  return 0;
}

int mca_set_descendents_require_layout_update(mc_node_list *node_list)
{
  for (int i = 0; i < node_list->count; ++i) {
    node_list->items[i]->layout->__requires_layout_update = true;

    if (node_list->items[i]->children) {
      MCcall(mca_set_descendents_require_layout_update(node_list->items[i]->children));
    }
  }

  return 0;
}

int mca_set_node_requires_layout_update(mc_node *node)
{
  // DEBUG?
  if (!node->layout) {
    MCerror(8416, "Can't set an update for a node with no layout");
  }

  // Set update required on all ancestors of the node
  while (node) {
    // DEBUG?
    if (!node->layout) {
      MCerror(8523, "Can't set an update for a node with a parent with no layout");
    }

    // Set
    node->layout->__requires_layout_update = true;

    // Move upwards through the ancestry
    node = node->parent;
  }

  return 0;
}

int mca_set_node_requires_rerender(mc_node *node)
{
  // DEBUG?
  if (!node->layout) {
    MCerror(8416, "Can't set an update for a node with no layout");
  }

  // Set update required on all ancestors of the node
  while (node) {
    // DEBUG?
    if (!node->layout) {
      MCerror(8523, "Can't set an update for a node with a parent with no layout");
    }

    // Set
    node->layout->__requires_rerender = true;
    // printf("'%s' __requires_rerender\n", node->name);

    // Move upwards through the ancestry
    node = node->parent;
  }

  return 0;
}

int mca_render_typical_nodes_children_present(image_render_details *image_render_queue, mc_node_list *children)
{
  mc_node *child;
  for (int a = 0; a < children->count; ++a) {
    child = children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_present(image_render_queue, child);
    }
  }
  return 0;
}

int mca_focus_node(mc_node *node)
{
  // Set layout update required on all ancestors of the node
  while (node) {
    // DEBUG?
    if (!node->layout) {
      MCerror(8523, "Can't set an update for a node with a parent with no layout");
    }

    // Set
    node->layout->__requires_rerender = true;

    if (!node->parent) {
      break;
    }
    if (!node->parent->layout) {
      MCerror(9664, "Cannot set focus to node with an ancestor without an initialized layout");
    }

    node->parent->layout->focused_child = node;

    // Find the child in the parents children and set it to the highest index amongst its z-index equals or lessers
    mc_node_list *parents_children = node->parent->children;
    // DEBUG CHECK
    bool found = false;
    for (int i = 0; i < parents_children->count; ++i) {
      if (parents_children->items[i] == node) {
        found = true;

        if (i + 1 == parents_children->count)
          break;

        int j = i + 1;
        for (; j < parents_children->count; ++j) {
          if (parents_children->items[j]->layout &&
              node->layout->z_layer_index < parents_children->items[j]->layout->z_layer_index) {
            break;
          }
        }
        --j;

        if (j > i) {
          // Change Child Order
          for (int k = i; k < j; ++k) {
            parents_children->items[k] = parents_children->items[k + 1];
          }

          parents_children->items[j] = node;
        }
      }
    }

    // Move upwards through the ancestry
    node = node->parent;
  }

  return 0;
}

int mca_obtain_focused_node(mc_node **node)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  *node = app_info->global_node;

  while ((*node)->layout && (*node)->layout->focused_child) {
    (*node) = (*node)->layout->focused_child;
  }

  return 0;
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

int mca_register_event_handler(mc_app_event_type event_type, void *handler_delegate, void *handler_state)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // printf("app_info->event_handlers.count:%u\n", app_info->event_handlers.count);
  event_handler_array *eha = app_info->event_handlers.items[event_type];

  event_handler_info *eh = (event_handler_info *)malloc(sizeof(event_handler_info));
  eh->delegate = handler_delegate;
  eh->state = handler_state;

  // printf("eha->handlers:%p, eha->capacity:%u\n", eha->handlers, eha->capacity);
  MCcall(append_to_collection((void ***)&eha->handlers, &eha->capacity, &eha->count, eh));

  return 0;
}

int mca_fire_event(mc_app_event_type event_type, void *event_arg)
{
  // printf("mca_fire_event:%i\n", event_type);
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  event_handler_array *eha = app_info->event_handlers.items[event_type];
  for (int a = 0; a < eha->count; ++a) {
    // puts("mca_fire_event_execute");
    int (*event_handler)(void *, void *) = (int (*)(void *, void *))eha->handlers[a]->delegate;
    MCcall(event_handler(eha->handlers[a]->state, event_arg));
  }

  return 0;
}

int mca_register_loaded_project(mc_project_info *project)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  MCcall(append_to_collection((void ***)&app_info->projects.items, &app_info->projects.capacity,
                              &app_info->projects.count, project));

  // Fire an event...
  MCcall(mca_fire_event(MC_APP_EVENT_PROJECT_LOADED, (void *)project));

  return 0;
}
// {
//   // append_to_collection((void ***)&parent_attachment->children, &parent_attachment->children_alloc,
//   //                      &parent_attachment->child_count, node_to_add);

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