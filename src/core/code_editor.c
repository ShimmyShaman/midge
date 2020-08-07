#include "core/midge_core.h"

void build_code_editor()
{
  printf("build_code_editor()\n");

  // Build the function editor window
  // Instantiate: node global;
  mc_node_v1 *fedit = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  fedit->name = "code_editor";
  fedit->parent = command_hub->global_node;
  fedit->type = NODE_TYPE_VISUAL;

  fedit->data.visual.bounds.x = 298;
  fedit->data.visual.bounds.y = 40;
  fedit->data.visual.bounds.width = APPLICATION_SET_WIDTH - 298;
  fedit->data.visual.bounds.height = APPLICATION_SET_HEIGHT - fedit->data.visual.bounds.y;
  fedit->data.visual.image_resource_uid = 0;
  fedit->data.visual.requires_render_update = true;
  fedit->data.visual.render_delegate = &code_editor_render;
  fedit->data.visual.visible = false;
  fedit->data.visual.input_handler = &code_editor_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fedit));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;
  // printf("bce-d\n");

  // Code Lines
  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)malloc(sizeof(mc_code_editor_state_v1));
  // printf("state:'%p'\n", state);
  state->visual_node = fedit;
  state->in_view_function_live_debugger = false;
  state->font_resource_uid = 0;
  state->cursor.line = 0;
  state->cursor.col = 0;
  state->cursor.zen_col = 0;
  state->cursor.rtf_index = 0;
  state->cursor.requires_render_update = false;
  state->line_display_offset = 0;
  // state->text = (mc_cstring_list_v1 *)malloc(sizeof(mc_cstring_list_v1));
  // state->text->lines_alloc = 8;
  // state->text->lines = (char **)calloc(sizeof(char *), state->text->lines_alloc);
  // state->text->lines_count = 0;
  state->render_lines = (rendered_code_line **)malloc(sizeof(rendered_code_line *) * CODE_EDITOR_RENDERED_CODE_LINES);
  // state->rendered_line_count = CODE_EDITOR_RENDERED_CODE_LINES;

  {
    // Source
    state->source_data = NULL;
    state->code.syntax = NULL;
    init_c_str(&state->code.rtf);
    state->code.syntax_updated = false;
  }

  {
    // FunctionLiveDebug
    state->fld_view = (fld_view_state *)malloc(sizeof(fld_view_state));
    state->fld_view->declare_incremental_uid = 0;
    state->fld_view->arguments.alloc = 0;
    state->fld_view->arguments.count = 0;
    state->fld_view->visual_code.alloc = 0;
    state->fld_view->visual_code.count = 0;
  }

  // printf("bce-s\n");
  for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
    state->render_lines[i] = (rendered_code_line *)malloc(sizeof(rendered_code_line));

    state->render_lines[i]->index = i;
    state->render_lines[i]->requires_render_update = true;
    MCcall(init_c_str(&state->render_lines[i]->rtf));
    // MCcall(set_c_str(state->render_lines[i]->rtf, ""));
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
  // printf("bce-w\n");

  {
    // Status Bar
    state->status_bar.bounds.x = 2;
    state->status_bar.bounds.y = fedit->data.visual.bounds.height - 50;
    state->status_bar.bounds.width = fedit->data.visual.bounds.width - 4;
    state->status_bar.bounds.height = 48;
    state->status_bar.image_resource_uid = 0;
    state->status_bar.message = NULL;
    state->status_bar.requires_render_update = true;

    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &state->status_bar.image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = state->status_bar.bounds.width;
    command->data.create_texture.height = state->status_bar.bounds.height;
  }

  {
    // Suggestion Box
    state->suggestion_box.visible = false;
    state->suggestion_box.requires_render_update = false;

    state->suggestion_box.selected_index = 0;
    state->suggestion_box.entries.count = 0;
    state->suggestion_box.entries.max_count = 8;
    state->suggestion_box.entries.items = (char **)malloc(sizeof(char *) * state->suggestion_box.entries.max_count);
    for (int a = 0; a < state->suggestion_box.entries.max_count; ++a) {
      state->suggestion_box.entries.items[a] = NULL;
    }
    state->suggestion_box.bounds.x = 2;
    state->suggestion_box.bounds.y = 2;
    state->suggestion_box.bounds.width = 280;
    state->suggestion_box.bounds.height = 12 + 8 * 24;
    state->suggestion_box.image_resource_uid = 0;

    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &state->suggestion_box.image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = state->suggestion_box.bounds.width;
    command->data.create_texture.height = state->suggestion_box.bounds.height;
  }

  mc_process_action_arg_info_v1 **context_args =
      (mc_process_action_arg_info_v1 **)malloc(sizeof(mc_process_action_arg_info_v1 *) * 1);
  context_args[0] = (mc_process_action_arg_info_v1 *)malloc(sizeof(mc_process_action_arg_info_v1));
  context_args[0]->name = "context_syntax_node";
  context_args[0]->type_name = "int";

  mc_process_action_arg_info_v1 **result_args =
      (mc_process_action_arg_info_v1 **)malloc(sizeof(mc_process_action_arg_info_v1 *) * 1);
  result_args[0] = (mc_process_action_arg_info_v1 *)malloc(sizeof(mc_process_action_arg_info_v1));
  result_args[0]->name = "output";
  result_args[0]->type_name = "char *";

  construct_process_action_database(2424, &state->entry_pad, "Code-Editor:User-Key-Type", 1, context_args, 1,
                                    result_args);

  // state->editor_button_panel.alloc = 0;
  // state->editor_button_panel.count = 0;
  // for (int i = 0; i < 2; ++i) {
  //   editor_panel_button_info *button = (editor_panel_button_info *)malloc(sizeof(editor_panel_button_info));

  //   button->bounds.max_width = 160;
  //   button->bounds.max_height = 24;

  //   MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  //   command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  //   command->p_uid = &button->image_resource_uid;
  //   command->data.create_texture.use_as_render_target = true;
  //   command->data.create_texture.width = fedit->data.visual.bounds.max_width;
  //   command->data.create_texture.height = fedit->data.visual.bounds.max_height;
  // }

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
  // printf("bce-z\n");
  // printf("41cestate->code.syntax=%p\n", state->code.syntax);
}
