#include "core/midge_core.h"



[Function][InitializeOnLoad] void init_function_live_debugger()
{
  node *fld = (node *)malloc(sizeof(node));
  fld->name = "function_live_debugger";
  fld->parent = command_hub->global_node;
  fld->type = NODE_TYPE_VISUAL;

  fld->data.visual.bounds.x = 400;
  fld->data.visual.bounds.y = 100;
  fld->data.visual.bounds.width = 800;
  fld->data.visual.bounds.height = 600;
  fld->data.visual.image_resource_uid = 0;
  fld->data.visual.requires_render_update = true;
  fld->data.visual.render_delegate = &function_live_debugger_render_v1;
  fld->data.visual.hidden = true;
  fld->data.visual.input_handler = &function_live_debugger_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fld));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  //   // Code Lines
  //   mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)malloc(sizeof(mc_code_editor_state_v1));
  //   // printf("state:'%p'\n", state);
  //   state->source_data_type = CODE_EDITOR_SOURCE_DATA_NONE;
  //   state->source_data = NULL;
  //   state->font_resource_uid = 0;
  //   state->cursorLine = 0;
  //   state->cursorCol = 0;
  //   state->line_display_offset = 0;
  //   state->text = (mc_cstring_list_v1 *)malloc(sizeof(mc_cstring_list_v1));
  //   state->text->lines_alloc = 8;
  //   state->text->lines = (char **)calloc(sizeof(char *), state->text->lines_alloc);
  //   state->text->lines_count = 0;
  //   state->render_lines = (rendered_code_line **)malloc(sizeof(rendered_code_line *) *
  //   CODE_EDITOR_RENDERED_CODE_LINES);

  //   for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
  //     state->render_lines[i] = (rendered_code_line *)malloc(sizeof(rendered_code_line));

  //     state->render_lines[i]->index = i;
  //     state->render_lines[i]->requires_render_update = true;
  //     state->render_lines[i]->text = NULL;
  //     //  "!this is twenty nine letters! "
  //     //  "!this is twenty nine letters! "
  //     //  "!this is twenty nine letters! ";

  //     MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  //     command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  //     command->p_uid = &state->render_lines[i]->image_resource_uid;
  //     command->data.create_texture.use_as_render_target = true;
  //     command->data.create_texture.width = state->render_lines[i]->width = fedit->data.visual.bounds.width - 4;
  //     command->data.create_texture.height = state->render_lines[i]->height = 28;
  //   }
  //   fedit->extra = (void *)state;

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &fld->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = fld->data.visual.bounds.width;
  command->data.create_texture.height = fld->data.visual.bounds.height;

  // MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  // command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  // command->p_uid = &console->input_line.image_resource_uid;
  // command->data.create_texture.use_as_render_target = true;
  // command->data.create_texture.width = console->input_line.width;
  // command->data.create_texture.height = console->input_line.height;

//   MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
//   command->type = RESOURCE_COMMAND_LOAD_FONT;
//   command->p_uid = &state->font_resource_uid;
//   command->data.font.height = 20;
//   command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
}