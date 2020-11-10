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

#ifdef __cplusplus
extern "C" {
#endif

void mcc_initialize_input_state();

#ifdef __cplusplus
}
#endif

#endif // MC_CONTROLLER_H