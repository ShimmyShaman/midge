// /* stack_container.h */

// #ifndef stack_container_H
// #define stack_container_H

// #include "control/mc_controller.h"
// #include "core/core_definitions.h"
// #include "mc_str.h"
// #include "render/render_common.h"

// typedef enum mcu_stack_orientation {
//   MCU_STACK_ORIENTATION_NULL = 0,
//   MCU_STACK_ORIENTATION_VERTICAL,
//   MCU_STACK_ORIENTATION_HORIZONTAL,
// } mcu_stack_orientation;

// typedef struct mcu_stack_container {
//   mc_node *node;

//   void *tag;

//   mcu_stack_orientation orientation;
//   struct {
//     float width, height;
//   } _children_extents;

//   render_color background_color;
// } mcu_stack_container;

// int mcu_init_stack_container(mc_node *parent, mcu_stack_container **p_button);

// #endif // stack_container_H