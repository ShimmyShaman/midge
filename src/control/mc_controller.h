/* mc_controller.h */

#ifndef MC_CONTROLLER_H
#define MC_CONTROLLER_H

#include "platform/mc_xcb.h"
#include "render/render_thread.h"

typedef enum mci_button_state {
  BUTTON_STATE_NULL = 0,
  BUTTON_STATE_DOWN = 1,
  BUTTON_STATE_UP = 2,
  BUTTON_STATE_PRESSED = 4,
  BUTTON_STATE_RELEASED = 8,
} mci_button_state;

typedef struct mci_input_state {
  // Indicates the function state (any key that corresponds to their function,
  // not the individual keys)
  // Uses mci_button_state flags (but because of how C handles flags it has to be an int)
  int alt_function, shift_function, ctrl_function;

  int *kb_states;

  struct {
    // Cursor location
    int x, y;

    // Indicates the button state
    // Uses mci_button_state flags (but because of how C handles flags it has to be an int)
    int left, middle, right, aux_1, aux_2;
  } mouse;

} mci_input_state;

typedef struct mci_input_event {
  window_input_event_type type;
  int button_code;

  mci_input_state *input_state;

  bool handled;
} mci_input_event;

int mcc_initialize_input_state();
int mcc_input_state_activate_kb_state_tracking();
void mcc_update_xcb_input();

#endif // MC_CONTROLLER_H