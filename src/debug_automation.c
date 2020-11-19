#include "core/midge_core.h"

typedef struct debug_data_state {
  int sequenceStep;
  mc_node_v1 *core_display;
} debug_data_state;

void create_hello_world_console_app()
{
  // Create a console application that just prints to console
  printf("create_hello_world_console_app\n");

  // Create a node & add it to global
  mc_node_v1 *app_node = (mc_node_v1 *)calloc(sizeof(mc_node_v1), 1);

  allocate_and_copy_cstr(app_node->name, "hello-world-console");
  app_node->parent = command_hub->global_node;
  app_node->type = NODE_TYPE_CONSOLE_APP;

  console_app_info *app_info = (console_app_info *)malloc(sizeof(console_app_info));
  app_node->extra = app_info;

  // Create the initialize function
  const char *ibpa_code = "void initialize_button_print_app(node *p_node)\n"
                          "{\n"
                          "  printf(\"Hello World!\\n\");\n"
                          "}";
  // "  exit_app(p_node, 0);\n"

  mc_function_info_v1 *func_info;
  instantiate_definition_from_code(app_node, ibpa_code, &func_info); //(void **)
  app_info->initialize_app = func_info;

  attach_node_to_hierarchy(command_hub->global_node, app_node);

  // Export it
  MCcall(obtain_subnode_with_name(command_hub->global_node, "hello-world-console", &app_node));
  if (!app_node) {
    MCerror(47, "Could not it!");
  }

  export_node_to_application(app_node, "test");
}

void create_hello_world_visual_app()
{
  // Create a visual application that launches from the command-line
  // - has a textblock with text on it
  // - has a close button to close it
  printf("create_hello_world_visual_app\n");

  // Create a node & add it to global
  mc_node_v1 *app_node = (mc_node_v1 *)calloc(sizeof(mc_node_v1), 1);

  allocate_and_copy_cstr(app_node->name, "hello-world-console");
  app_node->parent = command_hub->global_node;
  app_node->type = NODE_TYPE_VISUAL_PROJECT;

  visual_app_info *app_info = (visual_app_info *)malloc(sizeof(visual_app_info));
  app_info->update_app = NULL;
  app_node->extra = app_info;

  // Create the initialize function
  const char *init_code = "void initialize_hello_world_visual_app(node *p_node)\n"
                          "{\n"
                          "  mcu_text_block *text_block;\n"
                          "  mcu_init_text_block(p_node, &text_block);\n"
                          "\n"
                          "  set_mc_str(text_block->text, \"Hello Universe\");\n"
                          "  text_block->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;\n"
                          "  text_block->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;\n"
                          "\n"
                          "  mcu_update_ui(p_node);\n"
                          "}";

  mc_function_info_v1 *func_info;
  instantiate_definition_from_code(app_node, init_code, &func_info); //(void **)
  app_info->initialize_app = func_info;

  attach_node_to_hierarchy(command_hub->global_node, app_node);

  // Export it
  MCcall(obtain_subnode_with_name(command_hub->global_node, "hello-world-visual", &app_node));
  if (!app_node) {
    MCerror(47, "Could not it!");
  }

  export_node_to_application(app_node, "test");
}

void debug_automation(frame_time *elapsed, debug_data_state *debugState)
{
  // printf("debugState:%p\n", debugState);
  // mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)fedit->extra;

  switch (debugState->sequenceStep) {
  case 0: {
    // Select
    ++debugState->sequenceStep;

    create_hello_world_visual_app();
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