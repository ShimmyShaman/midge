/* hierarchy.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "mc_error_handling.h"
#include "render/render_common.h"
#include "ui/ui_definitions.h"

#include "core/midge_app.h"

void mc_print_node_path(mc_node *node)
{
  if(!node) {
    printf("NULL");
    return;
  }

  if (node->parent) {
    mc_print_node_path(node->parent);
    printf(">");
  }
  printf("%s", node->name);
}

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

int mca_find_hierarchy_node_in_descendants(mc_node *ancestor, const char *target_node_path, mc_node **out_result) {
  int lgt = -1;
  const char *c = target_node_path;
  bool final_path = false;
  while(1) {
    if(*c == '\0' || *c == '>') {
      final_path = (*c == '\0');
      lgt = (int)(c - target_node_path);
      break;
    }
    ++c;
  }

  if(lgt == -1) {
    MCerror(8274, "Could not discern target node path:'%s'\n", target_node_path);
    *out_result = NULL;
    return 0;
  }

  if(!ancestor->children || !ancestor->children->count) {
    *out_result = NULL;
    printf("[2421] mca_find_hierarchy_node_in_descendants: node '%s' doesn't have any descendants\n", target_node_path);
  }

  for(int i = 0; i < ancestor->children->count; ++i) {
    mc_node *child = ancestor->children->items[i];
    if(!strncmp(child->name, target_node_path, lgt)) {
      if(final_path)
        *out_result = child;
      else {
        MCcall(mca_find_hierarchy_node_in_descendants(ancestor, target_node_path + lgt + 1, out_result));
      }

      return 0;
    }
  }

  char child_name[512];
  strncpy(child_name, target_node_path, lgt);
  printf("[2424] mca_find_hierarchy_node_in_descendants: couldn't find child '%s' from target_node_path='%s'\n", child_name, target_node_path);

  return 0;
}

int mca_find_hierarchy_node_any(mc_node *any_hierarchy_node, const char *target_node_path, mc_node **out_result) {
  printf("target_node_path:'%s'\n", target_node_path);
  mc_node *root = any_hierarchy_node;
  while(root->parent)
    root = root->parent;

  if(strcmp(root->name, MIDGE_ROOT_NODE_NAME)) {
    MCerror(9578, "Deepest ancestor of the arg any_hierarchy_node must be 'midge-root', but is instead '%s'", root->name);
  }

  if(!strcmp(root->name, target_node_path)) {
    // Result is just midge-app
    *out_result = root;
    return 0;
  }

  if(strncmp(root->name, target_node_path, 10)) {
    MCerror(9534, "start of the target node path should begin with 'midge-root'");
  }

  MCcall(mca_find_hierarchy_node_in_descendants(root, target_node_path + strlen(MIDGE_ROOT_NODE_NAME), out_result));

  return 0;
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

  MCcall(mca_fire_event(MC_APP_EVENT_HIERARCHY_UPDATED, node_to_attach));
  if(node_to_attach->layout && node_to_attach->layout->visible)
    MCcall(mca_fire_event(MC_APP_EVENT_VISUAL_HIERARCHY_UPDATED, node_to_attach));

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
  (*node)->destroy_data = NULL;
  (*node)->parent = NULL;

  return 0;
}

void mca_destroy_node(mc_node *node)
{
  if (node->children) {
    for (int a = 0; a < node->children->count; ++a) {
      mca_destroy_node(node->children->items[a]);
    }

    free(node->children);
  }

  if (node->destroy_data && node->data) {
    void (*destroy_data)(void *data) = (void (*)(void *))node->destroy_data;
    destroy_data(node->data);
  }
  else if (node->data) {
    // // TODO -- warnings atm
    // printf("Warning: Node without destroy_data impl:'");
    // mc_print_node_name(node);
    // puts("'");
  }

  if (node->name) {
    free(node->name);
  }

  if (node->layout) {
    free(node->layout);
  }

  free(node);
}

int mca_determine_typical_node_extents_halt_propagation(mc_node *node, layout_extent_restraints restraints)
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

  return 0;
}

int mca_determine_typical_node_extents(mc_node *node, layout_extent_restraints restraints)
{
  // DEBUG
  // Ensure only that which is updated gets called
  // printf("ExtentsDet:'");
  // mc_print_node_name(node);
  // printf("'\n");
  // DEBUG

  MCcall(mca_determine_typical_node_extents_halt_propagation(node, restraints));

  // Children
  if (node->children) {
    for (int a = 0; a < node->children->count; ++a) {
      mc_node *child = node->children->items[a];
      if (child->layout && child->layout->determine_layout_extents) {
        // TODO fptr casting
        void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
            (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
        determine_layout_extents(child, LAYOUT_RESTRAINT_NONE); // TODO -- does a child inherit its parents restraints?
      }
    }
  }

  return 0;
}

// Any modifications to this method should also be made to mca_update_typical_node_layout_partially below
// TODO -- make this return int success and all delegates etc
int mca_update_typical_node_layout(mc_node *node, mc_rectf const *available_area)
{
  // DEBUG
  // Ensure only that which is updated gets called
  // printf("LayoutUpdate:'");
  // mc_print_node_name(node);
  // printf("'\n");
  // DEBUG

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
  mc_node *child;
  mca_node_layout *child_layout;
  if (node->children) {
    for (int a = 0; a < node->children->count; ++a) {
      child = node->children->items[a];
      child_layout = child->layout;

      if (child_layout && child_layout->update_layout && child_layout->__requires_layout_update) {
        // TODO fptr casting
        void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child_layout->update_layout;
        update_layout(child, &layout->__bounds);
      }
    }
  }

  return 0;
}

// TODO -- make this return int success and all delegates etc
int mca_update_typical_node_layout_partially(mc_node *node, mc_rectf const *available_area, bool update_x,
                                             bool update_y, bool update_width, bool update_height, bool update_children)
{
  // Preferred value > padding (within min/max if set)
  mc_rectf bounds;
  mca_node_layout *layout = node->layout;
  layout->__requires_layout_update = false;

  if (update_width) {
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

    if (bounds.width != layout->__bounds.width) {
      layout->__bounds.width = bounds.width;
      mca_set_node_requires_rerender(node);
    }
  }

  if (update_height) {
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

    if (bounds.height != layout->__bounds.height) {
      layout->__bounds.height = bounds.height;
      mca_set_node_requires_rerender(node);
    }
  }

  if (update_x) {
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

    if (bounds.x != layout->__bounds.x) {
      layout->__bounds.x = bounds.x;
      mca_set_node_requires_rerender(node);
    }
  }

  if (update_y) {
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

    if (bounds.y != layout->__bounds.y) {
      layout->__bounds.y = bounds.y;
      mca_set_node_requires_rerender(node);
    }
  }

  // Children
  if (update_children && node->children) {
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

int mca_attach_to_ancestor_root(mc_node *ancestor, mc_node *node_to_attach)
{
  // Search up the ancestoral root of the given parent
  unsigned int ancestoral_marker;
  do {
    ancestoral_marker = *(unsigned int *)ancestor->data;

    if (ancestoral_marker == MIDGE_APP_INFO_ROOT_UID) {
      // Found
      break;
    }
  } while (ancestor = ancestor->parent);

  if (!ancestor) {
    MCerror(8657, "Could not find a root ancestor.");
  }

  // Insert it appropriately according to its z-index
  mc_node_list *children = ancestor->children;
  int a;
  mc_node *oc;
  for (a = children->count; a > 0; --a) {
    oc = children->items[a - 1];

    if (oc->layout && oc->layout->z_layer_index <= node_to_attach->layout->z_layer_index) {
      MCcall(insert_in_collection((void ***)&children->items, &children->alloc, &children->count, a, node_to_attach));
      break;
    }
  }
  if (!a) {
    MCcall(insert_in_collection((void ***)&children->items, &children->alloc, &children->count, a, node_to_attach));
  }
  node_to_attach->parent = ancestor;

  MCcall(mca_set_node_requires_layout_update(node_to_attach));
  if (node_to_attach->children) {
    MCcall(mca_set_descendents_require_layout_update(node_to_attach->children));
  }

  MCcall(mca_fire_event(MC_APP_EVENT_HIERARCHY_UPDATED, node_to_attach));
  if(node_to_attach->layout && node_to_attach->layout->visible)
    MCcall(mca_fire_event(MC_APP_EVENT_VISUAL_HIERARCHY_UPDATED, node_to_attach));

  return 0;
}

int mca_detach_from_parent(mc_node *node)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  for (int a = app_info->global_node->children->count; a >= 0; --a) {
    // Only add unique items
    if (node == app_info->global_node->children->items[a]) {
      MCcall(remove_from_collection((void ***)&app_info->global_node->children->items,
                                    &app_info->global_node->children->count, a));
      break;
    }
  }

  node->parent = NULL;

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
  // printf("mca_set_node_requires_layout_update node=%s%s%s->%s\n",
  //        (node->parent && node->parent->parent) ? node->parent->parent->name : "",
  //        (node->parent && node->parent->parent) ? "->" : "", node->parent ? node->parent->name : "", node->name);
  // midge_error_print_thread_stack_trace();

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

int mca_render_node_list_headless(render_thread_info *render_thread, mc_node_list *children)
{
  mc_node *child;
  for (int a = 0; a < children->count; ++a) {
    child = children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(render_thread_info *, mc_node *) =
          (void (*)(render_thread_info *, mc_node *))child->layout->render_headless;
      render_node_headless(render_thread, child);
    }
  }
  return 0;
}

int mca_render_node_list_present(image_render_details *image_render_queue, mc_node_list *children)
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

int mca_render_present_delegate(image_render_details *image_render_queue, mc_node *node)
{
  if (node->layout && node->layout->visible && node->layout->render_present) {
    // TODO fptr casting
    void (*render_node_present)(image_render_details *, mc_node *) =
        (void (*)(image_render_details *, mc_node *))node->layout->render_present;
    render_node_present(image_render_queue, node);
  }

  return 0;
}

int mca_render_container_node_headless(render_thread_info *render_thread, mc_node *node)
{
  if (node->children) {
    mca_render_node_list_headless(render_thread, node->children);
  }

  return 0;
}

int mca_render_container_node_present(image_render_details *image_render_queue, mc_node *node)
{
  if (node->children) {
    mca_render_node_list_present(image_render_queue, node->children);
  }

  return 0;
}

int mca_focus_node(mc_node *node_to_focus)
{
  printf("mca_focus_node():\n - node_to_focus:");
  mc_print_node_path(node_to_focus);
  puts("");

  if(!node_to_focus->parent) {
    if(node_to_focus->type == NODE_TYPE_GLOBAL_ROOT) {
      MCerror(8581, "TODO");
    } else {
      MCerror(8571, "Cannot focus node not attached to the midge hierarchy");
    }
  }

  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // Set Focus Status on all parent nodes
  mc_node *hnode = node_to_focus, *hn_parent;
  do {
    hn_parent = hnode->parent;

    // Set Child Focus
    if (!hn_parent->layout) {
      MCerror(9664, "Cannot set focus to node with an ancestor without an initialized layout");
    }
    hn_parent->layout->focused_child = hnode;

    // Update layout
    hn_parent->layout->__requires_rerender = true;
    
    // Find the child in the parents children and set it to the highest index amongst its z-index equals or lessers
    mc_node_list *parents_children = hn_parent->children;
    bool found = false;
    for (int i = 0; i < parents_children->count; ++i) {
      if (parents_children->items[i] == hnode) {
        found = true;

        if (i + 1 == parents_children->count)
          break;

        int j = i + 1;
        for (; j < parents_children->count; ++j) {
          if (parents_children->items[j]->layout &&
              hnode->layout->z_layer_index < parents_children->items[j]->layout->z_layer_index) {
            break;
          }
        }
        --j;

        if (j > i) {
          // Change Child Order
          for (int k = i; k < j; ++k) {
            parents_children->items[k] = parents_children->items[k + 1];
          }

          parents_children->items[j] = hnode;
        }
      }
    }

    if(!found) {
      puts("! Couldn't find node in parents children !");
      printf(" - node_to_focus:");
      mc_print_node_path(node_to_focus);
      puts("");
      printf(" - parent:");
      mc_print_node_path(hn_parent);
      puts("");
      printf(" - node:");
      mc_print_node_path(hnode);
      puts("");

      MCerror(5857, "Couldn't find node in parents children");
    }

    // Set the midge-root focus status information
    if(hn_parent->type == NODE_TYPE_GLOBAL_ROOT) {
      switch (hnode->type)
      {
      case NODE_TYPE_CONSOLE_APP:
      case NODE_TYPE_VISUAL_PROJECT:
        app_info->focus_status.project = hnode;
        app_info->focus_status.project_target = node_to_focus;
        break;
      case NODE_TYPE_MODULE_ROOT:
        app_info->focus_status.module = hnode;
        app_info->focus_status.module_target = node_to_focus;
        break;
      default:
      puts("******************");
      printf(" - parent:");
      mc_print_node_path(hn_parent);
      puts("");
      printf(" - node:");
      mc_print_node_path(hnode);
      puts("");
        MCerror(5857, "Nodes attached to global root should only be a project node or module node: '%i'", hnode->type);
      }
      break;
    }

    // Continue
    hnode = hn_parent;

  } while(1);

  printf(" - module_focus:");
  mc_print_node_path(app_info->focus_status.module_target);
  puts("");
  printf(" - project_focus:");
  mc_print_node_path(app_info->focus_status.project_target);
  puts("");

  return 0;
}

void mca_obtain_focused_node(mc_node **node)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  *node = app_info->global_node;

  while ((*node)->layout && (*node)->layout->focused_child) {
    (*node) = (*node)->layout->focused_child;
  }
}

int mca_register_event(const char *event_code)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // Obtain the handler array for the current event
  event_handler_array *eha = NULL;
  for(int a = 0; a < app_info->event_handlers.count; ++a) {
    if(!strcmp(event_code, app_info->event_handlers.items[a]->event_code)) {
      eha = app_info->event_handlers.items[a];
      break;
    }
  }
  if(eha) {
    // Already exists, return
    return 0;
  }

  eha = (event_handler_array *)malloc(sizeof(event_handler_array));
  eha->event_code = strdup(event_code);
  eha->capacity = eha->count = 0U;

  MCcall(append_to_collection((void ***)&app_info->event_handlers.items, &app_info->event_handlers.capacity,
    &app_info->event_handlers.count, eha));

  return 0;
}

int mca_register_event_handler(const char *event_code, void *handler_delegate, void *handler_state)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // Obtain the handler array for the current event
  event_handler_array *eha = NULL;
  for(int a = 0; a < app_info->event_handlers.count; ++a) {
    if(!strcmp(event_code, app_info->event_handlers.items[a]->event_code)) {
      eha = app_info->event_handlers.items[a];
      break;
    }
  }
  if(!eha) {
    MCcall(mca_register_event(event_code));
    
    for(int a = 0; a < app_info->event_handlers.count; ++a) {
      if(!strcmp(event_code, app_info->event_handlers.items[a]->event_code)) {
        eha = app_info->event_handlers.items[a];
        break;
      }
    }
    if(!eha) {
      MCerror(4811, "should exist, just created it");
    }
  }

  event_handler_info *eh = (event_handler_info *)malloc(sizeof(event_handler_info));
  eh->delegate = handler_delegate;
  eh->state = handler_state;

  // printf("eha->handlers:%p, eha->capacity:%u\n", eha->handlers, eha->capacity);
  MCcall(append_to_collection((void ***)&eha->handlers, &eha->capacity, &eha->count, eh));

  return 0;
}

int mca_fire_event(const char *event_code, void *event_arg)
{
  // TODO when this is made UI-thread-safe align mca_fire_event_and_release_data also
  // printf("mca_fire_event:%i\n", event_type);
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // Obtain the handler array for the current event
  event_handler_array *eha = NULL;
  for(int a = 0; a < app_info->event_handlers.count; ++a) {
    if(!strcmp(event_code, app_info->event_handlers.items[a]->event_code)) {
      eha = app_info->event_handlers.items[a];
      break;
    }
  }
  if(!eha) {
    MCerror(9582, "The event='%s' has not been registered first.", event_code);
    // puts("Warning: No event handlers for the given event");
    return 0;
  }

  for (int a = 0; a < eha->count; ++a) {
    // puts("mca_fire_event_execute");
    int (*event_handler)(void *, void *) = (int (*)(void *, void *))eha->handlers[a]->delegate;
    MCcall(event_handler(eha->handlers[a]->state, event_arg));
  }

  return 0;
}

int mca_fire_event_and_release_data(const char *event_code, void *event_arg, int release_count, ...)
{
  MCcall(mca_fire_event(event_code, event_arg));

  if (release_count) {
    puts("TODO -- mca_fire_event_and_release_data va_arg/va_list fix");
    // TODO -- make this work
    // va_list ptrs_list;
    // va_start(ptrs_list, release_count);
    // for (int a = 0; a < release_count; ++a) {
    //   void *ptr = va_arg(ptrs_list, void *);
    //   if (ptr) {
    //     printf("mca_fire_event_and_release_data: %p released\n", ptr);
    //     free(ptr);
    //   }
    // }
    // va_end(ptrs_list);
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