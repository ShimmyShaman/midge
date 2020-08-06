#include "core/midge_core.h"

typedef struct usage_data_interface_state {
  unsigned int font_resource_uid;
} usage_data_interface_state;

void build_usage_data_interface()
{
  mc_node_v1 *usage_data_interface = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  usage_data_interface->name = "usage_data_interface";
  usage_data_interface->parent = command_hub->global_node;
  usage_data_interface->type = NODE_TYPE_VISUAL;

  usage_data_interface->child_count = 0;
  usage_data_interface->children_alloc = 4;
  usage_data_interface->children = (mc_node_v1 **)malloc(sizeof(void *) * usage_data_interface->children_alloc);

  usage_data_interface->data.visual.bounds.x = 0;
  usage_data_interface->data.visual.bounds.y = 0;
  usage_data_interface->data.visual.bounds.width = 300;
  usage_data_interface->data.visual.bounds.height = 900;
  usage_data_interface->data.visual.image_resource_uid = 0;
  usage_data_interface->data.visual.requires_render_update = true;
  usage_data_interface->data.visual.render_delegate = &core_display_render;
  usage_data_interface->data.visual.hidden = false;
  usage_data_interface->data.visual.input_handler = &core_display_handle_input;

  usage_data_interface_state *state = (usage_data_interface_state *)malloc(sizeof(usage_data_interface_state));
  state->font_resource_uid = 0;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, usage_data_interface));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &usage_data_interface->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = usage_data_interface->data.visual.bounds.width;
  command->data.create_texture.height = usage_data_interface->data.visual.bounds.height;

  // Font
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &state->font_resource_uid;
  command->data.font.height = 18;
  command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
}

void register_user_action(uint origin, uint source_action, int context_argsc, void **context_argsv, uint *register_uid)
{
  printf("action from source:%u\n", source_action);
  *register_uid = 3;
}

void report_user_action_effect(uint register_uid, int effect_argsc, void **effect_argsv)
{

  printf("action_reported:%u\n", register_uid);
}