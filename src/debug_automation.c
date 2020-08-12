#include "core/midge_core.h"

typedef struct debug_data_state {
  int sequenceStep;
  mc_node_v1 *core_display;
} debug_data_state;

void debug_automation(frame_time *elapsed, debug_data_state *debugState)
{
  // mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)fedit->extra;

  switch (debugState->sequenceStep) {
  case 0: {
    // Select
    ++debugState->sequenceStep;

    MCcall(create_button_print_app());
  } break;
  case 1: {
    // // Select
    ++debugState->sequenceStep;

    // mc_input_event_v1 *sim = (mc_input_event_v1 *)malloc(sizeof(mc_input_event_v1));
    // sim->type = INPUT_EVENT_MOUSE_PRESS;
    // sim->handled = false;
    // sim->shiftDown = false;
    // sim->altDown = false;
    // sim->ctrlDown = false;
    // sim->detail.mouse.button = MOUSE_BUTTON_LEFT;
    // sim->detail.mouse.x = 89;
    // sim->detail.mouse.y = 151;
    // {
    //   void *vargs[3];
    //   vargs[0] = argv[0];
    //   vargs[1] = &debugState->core_display;
    //   vargs[2] = &sim;
    //   MCcall(core_display_handle_input(3, vargs));
    // }

    // free(sim);
  } break;
  case 3: {
    // // Select
    ++debugState->sequenceStep;

    // mc_input_event_v1 *sim = (mc_input_event_v1 *)malloc(sizeof(mc_input_event_v1));
    // sim->type = INPUT_EVENT_MOUSE_PRESS;
    // sim->handled = false;
    // sim->shiftDown = false;
    // sim->altDown = false;
    // sim->ctrlDown = false;
    // sim->detail.mouse.button = MOUSE_BUTTON_LEFT;
    // sim->detail.mouse.x = 89;
    // sim->detail.mouse.y = 253;
    // {
    //   void *vargs[3];
    //   vargs[0] = argv[0];
    //   vargs[1] = &core_display;
    //   vargs[2] = &sim;
    //   MCcall(core_display_handle_input(3, vargs));
    // }

    // free(sim);
  } break;
  case 4: {
    // // Select
    ++debugState->sequenceStep;

    // mc_input_event_v1 *sim = (mc_input_event_v1 *)malloc(sizeof(mc_input_event_v1));
    // sim->type = INPUT_EVENT_MOUSE_PRESS;
    // sim->handled = false;
    // sim->shiftDown = false;
    // sim->altDown = false;
    // sim->ctrlDown = false;
    // sim->detail.mouse.button = MOUSE_BUTTON_LEFT;
    // sim->detail.mouse.x = 89;
    // sim->detail.mouse.y = 118;
    // {
    //   void *vargs[3];
    //   vargs[0] = argv[0];
    //   vargs[1] = &core_display;
    //   vargs[2] = &sim;
    //   MCcall(core_display_handle_input(3, vargs));
    // }

    // free(sim);
  } break;

  default:
    break;
  }

  return;
}

void begin_debug_automation()
{
  printf("feee\n");
  debug_data_state *debugState = (debug_data_state *)malloc(sizeof(debug_data_state));
  debugState->sequenceStep = 0;

  printf("ffwefw\n");
  debugState->core_display = NULL;
  MCcall(obtain_subnode_with_name(command_hub->global_node, CORE_OBJECTS_DISPLAY_NAME, &debugState->core_display));

  if (!debugState->core_display) {
    printf("ERROR DEBUG AUTO\n");
    return;
  }

  printf("bouts\n");
//   debug_automation(0, 0);
  printf("bboo %p\n", &debug_automation);
  register_update_timer(&debug_automation, 340 * 1000, true, (void *)debugState);
  printf("bun\n");

  return;
}