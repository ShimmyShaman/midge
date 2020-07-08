/* function_editor_handle_keyboard_input.c */

#include "core/midge_core.h"


void function_editor_handle_keyboard_input_v1(frame_time *elapsed, mc_node_v1 *fedit, mc_input_event_v1 *event)
{
//   function_editor_state *state = (function_editor_state *)fedit->extra;

  printf("key was %c\n", event->detail.keyboard.key);
}