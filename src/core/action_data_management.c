#include "core/midge_core.h"

typedef struct action_database_collection {
  uint count;
  uint alloc;
  mc_process_action_database_v1 **items;
} action_database_collection;

typedef struct usage_data_interface_state {
  mc_node_v1 *visual_node;
  bool minimized;

  uint alternate_image_resource_uid;
  mc_rect alternate_image_bounds;

  action_database_collection *databases;

  unsigned int font_resource_uid;
} usage_data_interface_state;

void usage_data_interface_render(frame_time *elapsed, mc_node_v1 *visual_node)
{
  image_render_queue *sequence;
  element_render_command *element_cmd;

  usage_data_interface_state *state = (usage_data_interface_state *)visual_node->extra;

  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->image_width = visual_node->data.visual.bounds.width;
  sequence->image_height = visual_node->data.visual.bounds.height;
  sequence->clear_color = COLOR_GHOST_WHITE;
  sequence->data.target_image.image_uid = visual_node->data.visual.image_resource_uid;

  if (state->minimized) {
    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    element_cmd->x = 2;
    element_cmd->y = 2;
    element_cmd->data.colored_rect_info.width = visual_node->data.visual.bounds.width - 4;
    element_cmd->data.colored_rect_info.height = visual_node->data.visual.bounds.height - 4;
    element_cmd->data.colored_rect_info.color = COLOR_GREEN;
  }
}

void init_usage_data_interface()
{
  printf("init_usage_data_interface\n");
  mc_node_v1 *usage_data_interface = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  usage_data_interface->name = "usage_data_interface";
  usage_data_interface->parent = command_hub->global_node;
  usage_data_interface->type = NODE_TYPE_VISUAL;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, usage_data_interface));

  usage_data_interface->child_count = 0;
  usage_data_interface->children_alloc = 4;
  usage_data_interface->children = (mc_node_v1 **)malloc(sizeof(void *) * usage_data_interface->children_alloc);

  usage_data_interface->data.visual.bounds.x = APPLICATION_SET_WIDTH - 100;
  usage_data_interface->data.visual.bounds.y = 2;
  usage_data_interface->data.visual.bounds.width = 100;
  usage_data_interface->data.visual.bounds.height = 38;
  usage_data_interface->data.visual.image_resource_uid = 0;
  usage_data_interface->data.visual.requires_render_update = true;
  usage_data_interface->data.visual.render_delegate = &usage_data_interface_render;
  usage_data_interface->data.visual.visible = true;
  usage_data_interface->data.visual.input_handler = NULL; //&core_display_handle_input;

  usage_data_interface_state *state = (usage_data_interface_state *)malloc(sizeof(usage_data_interface_state));
  usage_data_interface->extra = state;
  state->visual_node = usage_data_interface;

  state->minimized = true;
  state->alternate_image_resource_uid = 0;
  state->alternate_image_bounds.x = 298;
  state->alternate_image_bounds.y = 40;
  state->alternate_image_bounds.width = APPLICATION_SET_WIDTH - 298;
  state->alternate_image_bounds.height = APPLICATION_SET_HEIGHT - usage_data_interface->data.visual.bounds.y;
  state->databases = (action_database_collection *)malloc(sizeof(action_database_collection));
  state->databases->count = 0;
  state->databases->alloc = 0;
  state->databases->items = NULL;
  state->font_resource_uid = 0;

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Minimized Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &usage_data_interface->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = usage_data_interface->data.visual.bounds.width;
  command->data.create_texture.height = usage_data_interface->data.visual.bounds.height;

  // Maximized Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &state->alternate_image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = state->alternate_image_bounds.width;
  command->data.create_texture.height = state->alternate_image_bounds.height;

  // Font
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &state->font_resource_uid;
  command->data.font.height = 18;
  command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
}

void construct_process_action_database(uint owner_uid, mc_process_action_database_v1 **ptr_database, char *name,
                                       int context_argsc, mc_process_action_arg_info_v1 **context_args_details,
                                       int resultc, mc_process_action_arg_info_v1 **result_details)
{
  // Obtain the usage data interface state
  node *usage_data_interface;
  MCcall(obtain_subnode_with_name(command_hub->global_node, "usage_data_interface", &usage_data_interface));
  if (usage_data_interface == NULL) {
    printf("WARNING: Cannot find usage_data_interface\n");
    return;
  }
  usage_data_interface_state *state = (usage_data_interface_state *)usage_data_interface->extra;

  *ptr_database = (mc_process_action_database_v1 *)malloc(sizeof(mc_process_action_database_v1));
  (*ptr_database)->owner_uid = owner_uid;
  (*ptr_database)->action_uid_counter = 100;
  allocate_and_copy_cstr((*ptr_database)->name, name);
  (*ptr_database)->data.count = 0;
  (*ptr_database)->data.alloc = 0;
  (*ptr_database)->data.items = NULL;
  (*ptr_database)->context_args.count = context_argsc;
  (*ptr_database)->context_args.items = context_args_details;
  (*ptr_database)->result_args.count = resultc;
  (*ptr_database)->result_args.items = result_details;
  // printf("_result_type:%s\n", result_details[0]->type_name);

  MCcall(append_to_collection((void ***)&state->databases->items, &state->databases->alloc, &state->databases->count,
                              *ptr_database));
}

void register_user_action(mc_process_action_database_v1 *pad, uint source_action, int context_argsc,
                          void **context_data, uint *register_uid)
{
  // printf("[%p\n", pad);
  // printf("action from source-%s>%u\n", pad->name, source_action);
  mc_process_action_data_v1 *data_unit = (mc_process_action_data_v1 *)malloc(sizeof(mc_process_action_data_v1));
  data_unit->action_uid = pad->action_uid_counter++;
  data_unit->action_type = source_action;
  data_unit->context_data = context_data;
  data_unit->result_data = NULL;

  append_to_collection((void ***)&pad->data.items, &pad->data.alloc, &pad->data.count, data_unit);

  *register_uid = data_unit->action_uid;
}

void report_user_action_effect(mc_process_action_database_v1 *pad, uint register_uid, void **result_data)
{
  printf("action_reported:%u result:{", register_uid);
  if (!strcmp(pad->result_args.items[0]->type_name, "char *")) {
    printf("'%s'", (char *)result_data[0]);
  }
  else {
    printf("{unknown_result_type:%s", pad->result_args.items[0]->type_name);
  }
  printf("}\n");

  mc_process_action_data_v1 *data_unit = NULL;
  for (int i = pad->data.count - 1; i >= 0; --i) {
    if (pad->data.items[i]->action_uid != register_uid) {
      continue;
    }

    data_unit = pad->data.items[i];
    break;
  }

  if (!data_unit) {
    // TODO
    return;
  }

  data_unit->result_data = result_data;

  // Add to the recent actions list for this action

  // also to the recent globally tracked actions...TODO

  // Run any after-effect processes
  // If FUNCTION and result_data[0] == 'c' etc..
}