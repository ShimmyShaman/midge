#include "core/midge_core.h"

typedef struct debug_data_state {
  int sequenceStep;
  mc_node_v1 *core_display;
} debug_data_state;

void create_button_print_app()
{
  printf("create_button_print_app\n");

  // Create a node & add it to global
  mc_node_v1 *app_node = (mc_node_v1 *)calloc(sizeof(mc_node_v1), 1);
  add_node_as_child(command_hub->global_node, app_node);

  allocate_and_copy_cstr(app_node->name, "click-it");
  app_node->parent = command_hub->global_node;
  app_node->type = NODE_TYPE_PROJECT;

  

  // mc_node_v1 *button_node;
  // mgui_button_data *button_data;
  // mgui_create_button(app_node, &button, &button_data);
  // allocate_and_copy_cstr(button_data->text, "print it!");
  // button_data->margin...
  // button_data->size...
  // button_data->handler = &print_it_handler

  return;
}

void debug_automation(frame_time *elapsed, debug_data_state *debugState)
{
  // printf("debugState:%p\n", debugState);
  // mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)fedit->extra;

  switch (debugState->sequenceStep) {
  case 0: {
    // Select
    ++debugState->sequenceStep;

    create_button_print_app();
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
  debug_data_state *debugState = (debug_data_state *)malloc(sizeof(debug_data_state));
  debugState->sequenceStep = 0;

  // printf("debugState:%p\n", debugState);
  debugState->core_display = NULL;
  MCcall(obtain_subnode_with_name(command_hub->global_node, CORE_OBJECTS_DISPLAY_NAME, &debugState->core_display));

  if (!debugState->core_display) {
    printf("ERROR DEBUG AUTO\n");
    return;
  }

  // printf("bouts\n");
  //   debug_automation(0, 0);
  // printf("bboo %p\n", &debug_automation);
  register_update_timer(command_hub, &debug_automation, 340 * 1000, true, (void *)debugState);
  // printf("bun\n");

  return;
}