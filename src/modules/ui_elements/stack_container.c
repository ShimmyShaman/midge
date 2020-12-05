// /* stack_container.c */

// #include <stdio.h>
// #include <stdlib.h>

// #include "control/mc_controller.h"
// #include "core/core_definitions.h"
// #include "env/environment_definitions.h"

// #include "modules/ui_elements/stack_container.h"

// int _mcu_determine_stack_container_node_extents(mc_node *node, layout_extent_restraints restraints)
// {
//   // Data
//   mcu_stack_container *stack_container = (mcu_stack_container *)node->data;

//   // Apply Orientation
//   layout_extent_restraints child_restraints;
//   switch (stack_container->orientation) {
//   case MCU_STACK_ORIENTATION_HORIZONTAL:
//     child_restraints = LAYOUT_RESTRAINT_HORIZONTAL;
//     restraints |= LAYOUT_RESTRAINT_HORIZONTAL;
//     break;
//   case MCU_STACK_ORIENTATION_VERTICAL:
//     child_restraints = LAYOUT_RESTRAINT_VERTICAL;
//     restraints |= LAYOUT_RESTRAINT_VERTICAL;
//     break;
//   default:
//     MCerror(9018, "Unsupported stack orientation type : %i", stack_container->orientation);
//     break;
//   }

//   // Children
//   puts("update");
//   stack_container->_children_extents.width = stack_container->_children_extents.height = 0;
//   if (node->children) {
//     // Determine child extents
//     for (int a = 0; a < node->children->count; ++a) {
//       mc_node *child = node->children->items[a];
//       if (child->layout && child->layout->determine_layout_extents) {
//         // TODO fptr casting
//         void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
//             (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
//         determine_layout_extents(child, child_restraints);

//         printf("child det-ext: %.3f %.3f\n", child->layout->determined_extents.width,
//                child->layout->determined_extents.height);

//         // Propagate to the stack_container
//         if (stack_container->orientation == MCU_STACK_ORIENTATION_HORIZONTAL) {
//           stack_container->_children_extents.width += child->layout->determined_extents.width;
//         }
//         else if (child->layout->determined_extents.width > stack_container->_children_extents.width) {
//           stack_container->_children_extents.width = child->layout->determined_extents.width;
//         }
//         if (stack_container->orientation == MCU_STACK_ORIENTATION_VERTICAL) {
//           stack_container->_children_extents.height += child->layout->determined_extents.height;
//         }
//         else if (child->layout->determined_extents.height > stack_container->_children_extents.height) {
//           stack_container->_children_extents.height = child->layout->determined_extents.height;
//         }
//       }
//     }
//   }

//   // Determine Extents
//   const float MAX_EXTENT_VALUE = 100000.f;
//   mca_node_layout *layout = node->layout;

//   // -- Width
//   if (layout->preferred_width) {
//     // Set to preferred width
//     layout->determined_extents.width = layout->preferred_width;
//   }
//   else {
//     if (restraints & LAYOUT_RESTRAINT_HORIZONTAL) {
//       layout->determined_extents.width = stack_container->_children_extents.width;

//       if (layout->determined_extents.width < layout->min_width)
//         layout->determined_extents.width = layout->min_width;
//     }
//     else {
//       // padding adjusted from available
//       layout->determined_extents.width = MAX_EXTENT_VALUE;

//       // Specified bounds
//       if (layout->min_width && layout->determined_extents.width < layout->min_width) {
//         layout->determined_extents.width = layout->min_width;
//       }
//       if (layout->max_width && layout->determined_extents.width > layout->max_width) {
//         layout->determined_extents.width = layout->max_width;
//       }

//       if (layout->determined_extents.width < 0) {
//         layout->determined_extents.width = 0;
//       }
//     }
//   }

//   // -- Height
//   if (layout->preferred_height) {
//     // Set to preferred height
//     layout->determined_extents.height = layout->preferred_height;
//   }
//   else {
//     if (restraints & LAYOUT_RESTRAINT_VERTICAL) {
//       layout->determined_extents.height = stack_container->_children_extents.height;

//       if (layout->determined_extents.height < layout->min_height)
//         layout->determined_extents.height = layout->min_height;
//     }
//     else {
//       // padding adjusted from available
//       layout->determined_extents.height = MAX_EXTENT_VALUE;

//       // Specified bounds
//       if (layout->min_height && layout->determined_extents.height < layout->min_height) {
//         layout->determined_extents.height = layout->min_height;
//       }
//       if (layout->max_height && layout->determined_extents.height > layout->max_height) {
//         layout->determined_extents.height = layout->max_height;
//       }

//       if (layout->determined_extents.height < 0) {
//         layout->determined_extents.height = 0;
//       }
//     }
//   }

//   return 0;
// }

// int _mcu_update_stack_container_node_layout(mc_node *node, mc_rectf *available_area)
// {
//   // Data
//   mcu_stack_container *stack_container = (mcu_stack_container *)node->data;

//   // Clear
//   node->layout->__requires_layout_update = false;

//   // Preferred value > padding (within min/max if set)
//   mc_rectf bounds;
//   mca_node_layout *layout = node->layout;
//   layout->__requires_layout_update = false;

//   // Width
//   if (layout->preferred_width) {
//     // Set to preferred width
//     bounds.width = layout->preferred_width;
//   }
//   else {
//     // padding adjusted from available
//     bounds.width = available_area->width - layout->padding.right - layout->padding.left;

//     // Apply the determined extent
//     if (bounds.width > layout->determined_extents.width) {
//       bounds.width = layout->determined_extents.width;
//     }
//   }

//   // Height
//   if (layout->preferred_height) {
//     // Set to preferred height
//     bounds.height = layout->preferred_height;
//     // printf("preferred\n");
//   }
//   else {
//     printf("SC-available_area->height:%.3f layout->padding.bottom:%.3f layout->padding.top:%.3f "
//            "layout->determined_extents.height:%.3f\n",
//            available_area->height, layout->padding.bottom, layout->padding.top, layout->determined_extents.height);
//     // padding adjusted from available
//     bounds.height = available_area->height - layout->padding.bottom - layout->padding.top;

//     // Apply the determined extent

//     if (bounds.height > layout->determined_extents.height) {
//       bounds.height = layout->determined_extents.height;
//     }
//   }

//   // X
//   switch (layout->horizontal_alignment) {
//   case HORIZONTAL_ALIGNMENT_LEFT: {
//     // printf("left %.3f %.3f\n", available_area->x, layout->padding.left);
//     bounds.x = available_area->x + layout->padding.left;
//   } break;
//   case HORIZONTAL_ALIGNMENT_RIGHT: {
//     // printf("right %.3f %.3f %.3f %.3f\n", available_area->x, layout->padding.left, layout->padding.right,
//     // bounds.width);
//     bounds.x = available_area->x + available_area->width - layout->padding.right - bounds.width;
//   } break;
//   case HORIZONTAL_ALIGNMENT_CENTRED: {
//     // printf("centred %.3f %.3f %.3f %.3f %.3f\n", available_area->x, layout->padding.left, available_area->width,
//     //  layout->padding.right, bounds.width);
//     bounds.x = available_area->x + layout->padding.left +
//                (available_area->width - (layout->padding.left + bounds.width + layout->padding.right)) / 2.f;
//   } break;
//   default:
//     MCerror(7371, "NotSupported:%i", layout->horizontal_alignment);
//   }

//   // Y
//   switch (layout->vertical_alignment) {
//   case VERTICAL_ALIGNMENT_TOP: {
//     bounds.y = available_area->y + layout->padding.top;
//   } break;
//   case VERTICAL_ALIGNMENT_BOTTOM: {
//     bounds.y = available_area->y + available_area->height - layout->padding.bottom - bounds.height;
//   } break;
//   case VERTICAL_ALIGNMENT_CENTRED: {
//     bounds.y = available_area->y + layout->padding.top +
//                (available_area->height - (layout->padding.bottom + bounds.height + layout->padding.top)) / 2.f;
//   } break;
//   default:
//     MCerror(7387, "NotSupported:%i", layout->vertical_alignment);
//   }

//   // Set if different
//   if (bounds.x != layout->__bounds.x || bounds.y != layout->__bounds.y || bounds.width != layout->__bounds.width ||
//       bounds.height != layout->__bounds.height) {
//     layout->__bounds = bounds;
//     // printf("setrerender\n");
//     mca_set_node_requires_rerender(node);
//   }

//   // Children
//   if (node->children) {
//     // TODO -- test this some-time
//     // Adjust the bounds for stack_container differentations from children extents
//     if (bounds.width > stack_container->_children_extents.width)
//       bounds.x += (bounds.height - stack_container->_children_extents.width) / 2;
//     if (bounds.height > stack_container->_children_extents.height)
//       bounds.y += (bounds.height - stack_container->_children_extents.height) / 2;

//     for (int a = 0; a < node->children->count; ++a) {
//       mc_node *child = node->children->items[a];
//       if (child->layout && child->layout->update_layout) {
//         // TODO fptr casting
//         void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
//         update_layout(child, &bounds);

//         switch (stack_container->orientation) {
//         case MCU_STACK_ORIENTATION_HORIZONTAL:
//           bounds.x += child->layout->__bounds.width;
//           break;
//         case MCU_STACK_ORIENTATION_VERTICAL:
//           bounds.y += child->layout->__bounds.height;
//           break;
//         default:
//           MCerror(9245, "Unsupported stack orientation type : %i", stack_container->orientation);
//           break;
//         }
//       }
//     }
//   }

//   return 0;
// }

// void _mcu_render_stack_container_present(image_render_details *image_render_queue, mc_node *node)
// {
//   // Data
//   mcu_stack_container *stack_container = (mcu_stack_container *)node->data;

//   printf("stack_container] x:%f y:%f width:%f height:%f\n", node->layout->__bounds.x, node->layout->__bounds.y,
//          node->layout->__bounds.width, node->layout->__bounds.height);

//   // Background
//   mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
//                                         (unsigned int)node->layout->__bounds.y,
//                                         (unsigned int)node->layout->__bounds.width,
//                                         (unsigned int)node->layout->__bounds.height, stack_container->background_color);

//   if (node->children && node->children->count) {
//     mca_render_typical_nodes_children_present(image_render_queue, node->children);
//   }
// }

// int mcu_init_stack_container(mc_node *parent, mcu_stack_container **p_stack_container)
// {
//   // Node
//   mc_node *node;
//   MCcall(mca_init_mc_node(NODE_TYPE_DOESNT_MATTER, "unnamed-stack_container", &node));
//   node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
//   node->children->count = 0;
//   node->children->alloc = 0;

//   // Layout
//   MCcall(mca_init_node_layout(&node->layout));
//   node->layout->determine_layout_extents = (void *)&_mcu_determine_stack_container_node_extents;
//   node->layout->update_layout = (void *)&_mcu_update_stack_container_node_layout;
//   node->layout->render_headless = NULL;
//   node->layout->render_present = (void *)&_mcu_render_stack_container_present;
//   node->layout->handle_input_event = NULL; //(void *)&_mcu_stack_container_handle_input_event;

//   // Default Settings
//   node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
//   node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

//   // Control
//   mcu_stack_container *stack_container =
//       (mcu_stack_container *)malloc(sizeof(mcu_stack_container)); // TODO -- malloc check
//   stack_container->node = node;
//   node->data = stack_container;
//   stack_container->tag = NULL;

//   // Data Default Settings
//   stack_container->orientation = MCU_STACK_ORIENTATION_VERTICAL;
//   stack_container->background_color = COLOR_LIGHT_YELLOW;

//   // Set to out pointer
//   *p_stack_container = stack_container;

//   MCcall(mca_attach_node_to_hierarchy(parent, node));

//   return 0;
// }