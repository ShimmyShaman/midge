#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

int build_code_editor_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/
  // printf("bfe-a\n");

  // Build the function editor window
  // Instantiate: node global;
  mc_node_v1 *fedit = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  fedit->name = "code_editor";
  fedit->parent = command_hub->global_node;
  fedit->type = NODE_TYPE_VISUAL;

  fedit->data.visual.bounds.x = 298;
  fedit->data.visual.bounds.y = 40;
  fedit->data.visual.bounds.width = APPLICATION_SET_WIDTH - 300;
  fedit->data.visual.bounds.height = APPLICATION_SET_HEIGHT - 70;
  fedit->data.visual.image_resource_uid = 0;
  fedit->data.visual.requires_render_update = true;
  fedit->data.visual.render_delegate = &code_editor_render;
  fedit->data.visual.hidden = true;
  fedit->data.visual.input_handler = &code_editor_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fedit));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Code Lines
  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)malloc(sizeof(mc_code_editor_state_v1));
  // printf("state:'%p'\n", state);
  state->source_data_type = CODE_EDITOR_SOURCE_DATA_NONE;
  state->source_data = NULL;
  state->font_resource_uid = 0;
  state->cursorLine = 0;
  state->cursorCol = 0;
  state->line_display_offset = 0;
  state->text = (mc_cstring_list_v1 *)malloc(sizeof(mc_cstring_list_v1));
  state->text->lines_alloc = 8;
  state->text->lines = (char **)calloc(sizeof(char *), state->text->lines_alloc);
  state->text->lines_count = 0;
  state->render_lines = (rendered_code_line **)malloc(sizeof(rendered_code_line *) * CODE_EDITOR_RENDERED_CODE_LINES);

  for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
    state->render_lines[i] = (rendered_code_line *)malloc(sizeof(rendered_code_line));

    state->render_lines[i]->index = i;
    state->render_lines[i]->requires_render_update = true;
    state->render_lines[i]->text = NULL;
    //  "!this is twenty nine letters! "
    //  "!this is twenty nine letters! "
    //  "!this is twenty nine letters! ";

    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &state->render_lines[i]->image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = state->render_lines[i]->width = fedit->data.visual.bounds.width - 4;
    command->data.create_texture.height = state->render_lines[i]->height = 28;
  }
  fedit->extra = (void *)state;

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &fedit->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = fedit->data.visual.bounds.width;
  command->data.create_texture.height = fedit->data.visual.bounds.height;

  // MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  // command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  // command->p_uid = &console->input_line.image_resource_uid;
  // command->data.create_texture.use_as_render_target = true;
  // command->data.create_texture.width = console->input_line.width;
  // command->data.create_texture.height = console->input_line.height;

  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &state->font_resource_uid;
  command->data.font.height = 20;
  command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  return 0;
}