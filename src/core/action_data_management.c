#include "core/midge_core.h"

typedef struct usage_data_interface_state {
  bool minimized;

  uint alternate_image_resource_uid;
  mc_rect alternate_image_bounds;

  unsigned int font_resource_uid;
} usage_data_interface_state;

void usage_data_interface_render(frame_time *elapsed, mc_node_v1 *visual_node)
{
  image_render_queue *sequence;
  element_render_command *element_cmd;

  usage_data_interface_state *state = (usage_data_interface_state *)visual_node->extra;

  // MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  // sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  // sequence->image_width = visual_node->data.visual.bounds.width;
  // sequence->image_height = visual_node->data.visual.bounds.height;
  // sequence->clear_color = COLOR_GHOST_WHITE;
  // sequence->data.target_image.image_uid = visual_node->data.visual.image_resource_uid;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  // element_cmd->x = 2;
  // element_cmd->y = 2;
  // element_cmd->data.colored_rect_info.width = visual_node->data.visual.bounds.width - 4;
  // element_cmd->data.colored_rect_info.height = visual_node->data.visual.bounds.height - 4;
  // element_cmd->data.colored_rect_info.color = COLOR_DARK_SLATE_GRAY;
}

void build_usage_data_interface()
{
  mc_node_v1 *usage_data_interface = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  usage_data_interface->name = "usage_data_interface";
  usage_data_interface->parent = command_hub->global_node;
  usage_data_interface->type = NODE_TYPE_VISUAL;

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

  state->minimized = true;
  state->alternate_image_resource_uid = 0;
  state->alternate_image_bounds.x = 298;
  state->alternate_image_bounds.y = 40;
  state->alternate_image_bounds.width = APPLICATION_SET_WIDTH - 298;
  state->alternate_image_bounds.height = APPLICATION_SET_HEIGHT - usage_data_interface->data.visual.bounds.y;
  state->font_resource_uid = 0;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, usage_data_interface));

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

void register_user_action(uint origin, uint source_action, int context_argsc, void **context_argsv, uint *register_uid)
{
  printf("action from source:%u\n", source_action);
  *register_uid = 3;
}

void report_user_action_effect(uint register_uid, int effect_argsc, void **effect_argsv)
{

  printf("action_reported:%u\n", register_uid);
}