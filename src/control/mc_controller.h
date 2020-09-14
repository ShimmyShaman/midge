/* mc_controller.h */

#ifndef MC_CONTROLLER_H
#define MC_CONTROLLER_H

#include "platform/mc_xcb.h"
#include "render/render_thread.h"

typedef enum mci_key_state {
  KEY_STATE_NULL = 0,
  KEY_STATE_DOWN = 1,
  KEY_STATE_UP = 2,
  KEY_STATE_PRESSED = 4,
  KEY_STATE_RELEASED = 8,
} mci_key_state;

typedef struct mci_input_state {
  // Indicates the function state (any key that corresponds to their function,
  // not the individual keys)
  int alt_function, shift_function, ctrl_function;
} mci_input_state;

extern "C" {
void mcc_initialize_input_state();
}

#endif // MC_CONTROLLER_H